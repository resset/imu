/*
    IMU - Copyright (C) 2014-2021 Mateusz Tomaszkiewicz

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "icm20689.h"
#include "imu.h"

static binary_semaphore_t imu_ready_bsem;
static mutex_t imu_data_mtx;
imu_data_t imu_data;

const SPIConfig spicfg = {
  false,
  NULL,
  GPIOA,
  15,
  SPI_CFG1_MBR_DIV128 | SPI_CFG1_DSIZE_VALUE(7),
  SPI_CFG2_SSOE
};

inline static void imu_read(uint8_t *txbuf, size_t txbuf_len, uint8_t *rxbuf, size_t rxbuf_len)
{
  txbuf[0] |= ICM20689_READ_MASK;
  cacheBufferFlush(txbuf, CACHE_SIZE_ALIGN(uint8_t, txbuf_len));
  cacheBufferInvalidate(rxbuf, CACHE_SIZE_ALIGN(uint8_t, rxbuf_len));
  spiExchange(&SPID1, rxbuf_len, txbuf, rxbuf);
}

inline static void imu_write(uint8_t *txbuf, size_t txbuf_len)
{
  cacheBufferFlush(txbuf, CACHE_SIZE_ALIGN(uint8_t, txbuf_len));
  spiSend(&SPID1, txbuf_len, txbuf);
}

static int imu_init(void)
{
  CC_ALIGN_DATA(32) uint8_t txbuf[CACHE_SIZE_ALIGN(uint8_t, 2)];
  CC_ALIGN_DATA(32) uint8_t rxbuf[CACHE_SIZE_ALIGN(uint8_t, 2)];

  palSetPadMode(GPIOA, 15,
    PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_FLOATING); /* NSS */
  palSetPadMode(GPIOB,  3,
    PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_FLOATING); /* SCK */
  palSetPadMode(GPIOB,  4,
    PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_FLOATING); /* MISO */
  palSetPadMode(GPIOB,  5,
    PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUPDR_FLOATING); /* MOSI */

  spiAcquireBus(&SPID1);
  spiStart(&SPID1, &spicfg);

  spiSelect(&SPID1);

  /* Reset ICM20689.*/
  txbuf[0] = ICM20689_PWR_MGMT_1;
  txbuf[1] = 0x80;
  imu_write(txbuf, 2);
  chThdSleepMilliseconds(100);
  /* Here we check if the reset is done.*/
  do {
    rxbuf[0] = 0xff;
    txbuf[0] = ICM20689_PWR_MGMT_1;
    imu_read(txbuf, 1, rxbuf, 2);
  } while (rxbuf[0] & 0x80);
  /* Last step is to reset analog to digital paths of all sensors.*/
  txbuf[0] = ICM20689_SIGNAL_PATH_RESET;
  txbuf[1] = 0x07;
  imu_write(txbuf, 2);
  chThdSleepMilliseconds(100);

  /* Disable I2C interface and FIFO.*/
  txbuf[0] = ICM20689_USER_CTRL;
  txbuf[1] = 0x10;
  imu_write(txbuf, 2);
  txbuf[0] = ICM20689_FIFO_EN;
  txbuf[1] = 0x00;
  imu_write(txbuf, 2);

  /* Set clock source to gyro X.*/
  txbuf[0] = ICM20689_PWR_MGMT_1;
  txbuf[1] = 0x01;
  imu_write(txbuf, 2);

  /* Disable standby modes.*/
  txbuf[0] = ICM20689_PWR_MGMT_2;
  txbuf[1] = 0x00;
  imu_write(txbuf, 2);

  /* Disable interrupts.*/
  txbuf[0] = ICM20689_INT_ENABLE;
  txbuf[1] = 0x00;
  imu_write(txbuf, 2);

  /* Set gyroscope sensitivity to +/- 1000 deg/s.*/
  txbuf[0] = ICM20689_GYRO_CONFIG;
  txbuf[1] = 0x10;
  imu_write(txbuf, 2);

  /* Set accelerometer sensitivity to +/- 8 g.*/
  txbuf[0] = ICM20689_ACCEL_CONFIG;
  txbuf[1] = 0x10;
  imu_write(txbuf, 2);

  /* Set low pass filter cutoff frequency (DLPF_CFG). We set 41 Hz for gyro.
   * NOTE: it is preferable not to use MPU's filter. External software
   * filter (eg. biquad) in embedded environment will have better performance.
   */
  txbuf[0] = ICM20689_CONFIG;
  txbuf[1] = 0x03;
  imu_write(txbuf, 2);

  /* Set data rate (if DLPF_CFG == 0 then 8 kHz is divided, otherwise 1 kHz).
   * Since we use LPF, our data rate is 1 kHz.
   */
  txbuf[0] = ICM20689_SMPLRT_DIV;
  txbuf[1] = 0x00;
  imu_write(txbuf, 2);

  /* Test for ICM20689.*/
  txbuf[0] = ICM20689_WHO_AM_I;
  imu_read(txbuf, 1, rxbuf, 2);

  spiUnselect(&SPID1);
  spiReleaseBus(&SPID1);

  /* This should be a check of registers written.*/
  if (rxbuf[0] == ICM20689_VAL_WHO_AM_I) {
    return 0;
  } else {
    /* TODO: register fatal error, quit task.*/
    return 1;
  }
}

static void imu_get_data(void)
{
  CC_ALIGN_DATA(32) uint8_t txbuf[CACHE_SIZE_ALIGN(uint8_t, 2)];
  CC_ALIGN_DATA(32) uint8_t rxbuf[CACHE_SIZE_ALIGN(uint8_t, 24)];
  /* TODO: this is not needed.*/
  memset(txbuf, 0xff, CACHE_SIZE_ALIGN(uint8_t, 2));

  spiAcquireBus(&SPID1);
  spiSelect(&SPID1);

  txbuf[0] = ICM20689_ACCEL_XOUT_H;
  /* FIXME: why do we have this offset of one byte?*/
  imu_read(txbuf, 1, rxbuf, 16);

  /* FIXME: get rid of this offest, somehow.*/
  imu_data.accel_x = (int16_t)(rxbuf[1] << 8 | rxbuf[2]);
  imu_data.accel_y = (int16_t)(rxbuf[3] << 8 | rxbuf[4]);
  imu_data.accel_z = (int16_t)(rxbuf[5] << 8 | rxbuf[6]);

  /* Temperature must be calculated.*/
  imu_data.temperature = (float)((int16_t)rxbuf[7] << 8 | rxbuf[8]) / 326.8f + 25.0f;

  imu_data.gyro_x = (int16_t)(rxbuf[9] << 8 | rxbuf[10]);
  imu_data.gyro_y = (int16_t)(rxbuf[11] << 8 | rxbuf[12]);
  imu_data.gyro_z = (int16_t)(rxbuf[13] << 8 | rxbuf[14]);

  spiUnselect(&SPID1);
  spiReleaseBus(&SPID1);
}

void imu_sync_init(void)
{
  chBSemWait(&imu_ready_bsem);
}

void imu_copy_data(imu_data_t *source, imu_data_t *target)
{
  chMtxLock(&imu_data_mtx);
  memcpy(target, source, sizeof(imu_data_t));
  chMtxUnlock(&imu_data_mtx);
}

THD_WORKING_AREA(waImu, IMU_THREAD_STACK_SIZE);
THD_FUNCTION(thImu, arg)
{
  (void)arg;

  chRegSetThreadName("thImu");
  chBSemObjectInit(&imu_ready_bsem, true);

  imu_init();
  chBSemSignal(&imu_ready_bsem);

  systime_t time = chVTGetSystemTime();
  while (true) {
    /* Fire the thread every 400 us (2.5 kHz). This is a maximum we can achieve
       for accelerometer, gyroscope and temperature data on ICM20689.*/
    time = chTimeAddX(time, TIME_US2I(400));
    imu_get_data();

    chThdSleepUntil(time);
  }
}

void shellcmd_imu(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "%6d %6d %6d      %6d %6d %6d      %d\r\n",
             imu_data.accel_x, imu_data.accel_y, imu_data.accel_z,
             imu_data.gyro_x, imu_data.gyro_y, imu_data.gyro_z,
             (int32_t)(imu_data.temperature * 100));
    chThdSleepMilliseconds(50);
  }
}
