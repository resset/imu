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

#include "mpu6050.h"
#include "i2c_sensors.h"
#include "gyro.h"

#define MPU6050_ADDR MPU6050_ADDR_LOW

typedef struct {
  int16_t accel_xout;
  int16_t accel_yout;
  int16_t accel_zout;
  int16_t gyro_xout;
  int16_t gyro_yout;
  int16_t gyro_zout;
  int16_t temp_out;
} gyro_data_t;

gyro_data_t gyro_data;

inline static void imu_transmit(uint8_t *txbuf, size_t txbuf_len, uint8_t *rxbuf, size_t rxbuf_len)
{
  cacheBufferFlush(txbuf, CACHE_SIZE_ALIGN(uint8_t, txbuf_len));
  cacheBufferInvalidate(rxbuf, CACHE_SIZE_ALIGN(uint8_t, rxbuf_len));
  i2cMasterTransmitTimeout(&I2CD1, MPU6050_ADDR, txbuf, txbuf_len, rxbuf, rxbuf_len, TIME_INFINITE);
}

inline static void imu_send(uint8_t *txbuf, size_t txbuf_len)
{
  cacheBufferFlush(txbuf, CACHE_SIZE_ALIGN(uint8_t, txbuf_len));
  i2cMasterTransmitTimeout(&I2CD1, MPU6050_ADDR, txbuf, txbuf_len, NULL, 0, TIME_INFINITE);
}

static int gyro_init(void)
{
  CC_ALIGN_DATA(32) uint8_t txbuf[CACHE_SIZE_ALIGN(uint8_t, 2)];
  CC_ALIGN_DATA(32) uint8_t rxbuf[CACHE_SIZE_ALIGN(uint8_t, 2)];

  i2c_sensors_init();
  i2cAcquireBus(&I2CD1);

  /* Reset MPU6050. Note: apparently this is needed for SPI connection in MPU6000 only,
   * but it shouldn't hurt on I2C either.
   */
  txbuf[0] = MPU6050_PWR_MGMT_1;
  txbuf[1] = 0x80;
  imu_send(txbuf, 2);
  chThdSleepMilliseconds(100);
  /* Here we check if the reset is done.*/
  do {
    rxbuf[0] = 0xff;
    txbuf[0] = MPU6050_PWR_MGMT_1;
    imu_transmit(txbuf, 1, rxbuf, 1);
  } while (rxbuf[0] & 0x80);
  /* Last step is to reset analog to digital paths of all sensors.*/
  txbuf[0] = MPU6050_SIGNAL_PATH_RESET;
  txbuf[1] = 0x07;
  imu_send(txbuf, 2);
  chThdSleepMilliseconds(100);

  /* Set clock source to gyro X.*/
  txbuf[0] = MPU6050_PWR_MGMT_1;
  txbuf[1] = 0x01;
  imu_send(txbuf, 2);

  /* Test for MPU6050.*/
  txbuf[0] = MPU6050_WHO_AM_I;
  imu_transmit(txbuf, 1, rxbuf, 1);

  i2cReleaseBus(&I2CD1);

  /* This should be a check of registers written.*/
  if (rxbuf[0] == 0x68) {
    return 0;
  } else {
    /* TODO: register fatal error, quit task.*/
    return 1;
  }
}

static void gyro_read(void)
{
  CC_ALIGN_DATA(32) uint8_t txbuf[CACHE_SIZE_ALIGN(uint8_t, 2)];
  CC_ALIGN_DATA(32) uint8_t rxbuf[CACHE_SIZE_ALIGN(uint8_t, 24)];

  i2cAcquireBus(&I2CD1);

  txbuf[0] = 0x3B;
  imu_transmit(txbuf, 1, rxbuf, 14);

  gyro_data.accel_xout = (int16_t)(rxbuf[0] << 8 | rxbuf[1]);
  gyro_data.accel_yout = (int16_t)(rxbuf[2] << 8 | rxbuf[3]);
  gyro_data.accel_zout = (int16_t)(rxbuf[4] << 8 | rxbuf[5]);

  /* Temperature in deg. C * 10 must be calculated.*/
  gyro_data.temp_out = (int16_t)((int16_t)rxbuf[6] << 8 | rxbuf[7]) / 34 + 365;

  gyro_data.gyro_xout = (int16_t)(rxbuf[8] << 8 | rxbuf[9]);
  gyro_data.gyro_yout = (int16_t)(rxbuf[10] << 8 | rxbuf[11]);
  gyro_data.gyro_zout = (int16_t)(rxbuf[12] << 8 | rxbuf[13]);

  i2cReleaseBus(&I2CD1);
}

THD_WORKING_AREA(waGyro, 128);
THD_FUNCTION(thGyro, arg)
{
  (void)arg;

  chRegSetThreadName("thGyro");

  gyro_init();

  while (true) {
    gyro_read();
    chThdSleepMilliseconds(321);
  }
}

void shellcmd_gyro(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "%6d %6d %6d   %6d %6d %6d   %d\r\n",
             gyro_data.accel_xout, gyro_data.accel_yout, gyro_data.accel_zout,
             gyro_data.gyro_xout, gyro_data.gyro_yout, gyro_data.gyro_zout,
             gyro_data.temp_out);
    chThdSleepMilliseconds(50);
  }
}
