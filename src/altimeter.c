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

#include "altimeter.h"

const I2CConfig i2ccfg = {
  STM32_TIMINGR_PRESC(0x0U) |
  STM32_TIMINGR_SCLDEL(0x9U) | STM32_TIMINGR_SDADEL(0x0U) |
  STM32_TIMINGR_SCLH(0x19U) | STM32_TIMINGR_SCLL(0x4BU),
  0,
  0
};

altimeter_data_t altimeter_data;

static void i2c_transmit(uint8_t *txbuf, size_t txbuf_len, uint8_t *rxbuf, size_t rxbuf_len)
{
  cacheBufferFlush(txbuf, CACHE_SIZE_ALIGN(uint8_t, txbuf_len));
  cacheBufferInvalidate(rxbuf, CACHE_SIZE_ALIGN(uint8_t, rxbuf_len));
  i2cMasterTransmitTimeout(&I2CD1, BMP280_ADDR, txbuf, txbuf_len, rxbuf, rxbuf_len, TIME_INFINITE);
}

static void i2c_send(uint8_t *txbuf, size_t txbuf_len)
{
  cacheBufferFlush(txbuf, CACHE_SIZE_ALIGN(uint8_t, txbuf_len));
  i2cMasterTransmitTimeout(&I2CD1, BMP280_ADDR, txbuf, txbuf_len, NULL, 0, TIME_INFINITE);
}

int altimeter_init(void)
{
  CC_ALIGN_DATA(32) uint8_t txbuf[CACHE_SIZE_ALIGN(uint8_t, 2)];
  CC_ALIGN_DATA(32) uint8_t rxbuf[CACHE_SIZE_ALIGN(uint8_t, 24)];

  /*
   * I2C initialization.
   */
  palSetPadMode(GPIOB, 6, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN); /* SCL */
  palSetPadMode(GPIOB, 7, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN); /* SDA */

  i2cStart(&I2CD1, &i2ccfg);

  i2cAcquireBus(&I2CD1);

  /* Force power-on reset.*/
  txbuf[0] = BMP280_RESET;
  txbuf[1] = BMP280_VAL_RESET;
  i2c_send(txbuf, 2);
  chThdSleepMilliseconds(100);

  /* Test for BMP280.*/
  txbuf[0] = BMP280_ID;
  i2c_transmit(txbuf, 1, rxbuf, 1);

  if (rxbuf[0] != BMP280_VAL_ID) {
    /* TODO: fatal error, quit task.*/
    return 1;
  }

  /* config register
     t_sb = 000 (0.5 ms)
     filter = 111 (x16)
     Bit 3 cannot be written, hence this read before the write.

     NOTE: we have to use aligned memory in i2c_transmit/i2c_send,
     this is why the operations are performed on rxbuf.
  */
  txbuf[0] = BMP280_CONFIG;
  i2c_transmit(txbuf, 1, rxbuf, 1);
  rxbuf[0] = rxbuf[0] & 0x03;
  txbuf[0] = BMP280_CONFIG;
  txbuf[1] = 0x1C | rxbuf[0];
  i2c_send(txbuf, 2);

  /* ctrl_meas register
     osrs_t = 010 (x2, 17-bit, 0.0025 deg. C)
     osrs_p = 101 (x16, 20-bit, 0.16 Pa)
     mode = 11 (normal)
     With settings like this and IIR filter coefficient equaling 16,
     output data rate is 26.3 Hz.
  */
  txbuf[0] = BMP280_CTRL_MEAS;
  txbuf[1] = 0x57;
  i2c_send(txbuf, 2);

  /* Calibration data readout.*/
  txbuf[0] = BMP280_DIG_T1_LSB;
  i2c_transmit(txbuf, 1, rxbuf, 24);
  altimeter_data.dig_T1 = ((uint16_t)rxbuf[1]) << 8 | rxbuf[0];
  altimeter_data.dig_T2 = ((uint16_t)rxbuf[3]) << 8 | rxbuf[2];
  altimeter_data.dig_T3 = ((uint16_t)rxbuf[5]) << 8 | rxbuf[4];
  altimeter_data.dig_P1 = ((uint16_t)rxbuf[7]) << 8 | rxbuf[6];
  altimeter_data.dig_P2 = ((uint16_t)rxbuf[9]) << 8 | rxbuf[8];
  altimeter_data.dig_P3 = ((uint16_t)rxbuf[11]) << 8 | rxbuf[10];
  altimeter_data.dig_P4 = ((uint16_t)rxbuf[13]) << 8 | rxbuf[12];
  altimeter_data.dig_P5 = ((uint16_t)rxbuf[15]) << 8 | rxbuf[14];
  altimeter_data.dig_P6 = ((uint16_t)rxbuf[17]) << 8 | rxbuf[16];
  altimeter_data.dig_P7 = ((uint16_t)rxbuf[19]) << 8 | rxbuf[18];
  altimeter_data.dig_P8 = ((uint16_t)rxbuf[21]) << 8 | rxbuf[20];
  altimeter_data.dig_P9 = ((uint16_t)rxbuf[23]) << 8 | rxbuf[22];

  i2cReleaseBus(&I2CD1);

  return 0;
}

static int32_t bmp280_compensate_temperature(altimeter_data_t *ad)
{
  int32_t var1, var2;

  var1 = ((((ad->t_adc >> 3) - ((int32_t)ad->dig_T1 << 1))) * ((int32_t)ad->dig_T2)) >> 11;
  var2 = (((((ad->t_adc >> 4) - ((int32_t)ad->dig_T1)) * ((ad->t_adc >> 4) - ((int32_t)ad->dig_T1))) >> 12)
          * ((int32_t)ad->dig_T3)) >> 14;
  ad->t_fine = var1 + var2;
  ad->temperature = (ad->t_fine * 5 + 128) >> 8;

  return ad->temperature;
}

static uint32_t bmp280_compensate_pressure(altimeter_data_t *ad)
{
  int64_t var1, var2, p;

  var1 = ((int64_t)ad->t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)ad->dig_P6;
  var2 = var2 + ((var1 * (int64_t)ad->dig_P5) << 17);
  var2 = var2 + (((int64_t)ad->dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)ad->dig_P3) >> 8) + ((var1 * (int64_t)ad->dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)ad->dig_P1) >> 33;
  if (var1 == 0) {
    ad->pressure = 0;
    return 0;
  }
  p = 1048576 - ad->p_adc;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)ad->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)ad->dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)ad->dig_P7) << 4);
  ad->pressure = (uint32_t)p;

  return ad->pressure;
}

static void altimeter_read(void)
{
  CC_ALIGN_DATA(32) uint8_t txbuf[CACHE_SIZE_ALIGN(uint8_t, 2)];
  CC_ALIGN_DATA(32) uint8_t rxbuf[CACHE_SIZE_ALIGN(uint8_t, 24)];

  i2cAcquireBus(&I2CD1);

  //chThdSleepMilliseconds(1000);
  txbuf[0] = BMP280_PRESS_MSB;
  i2c_transmit(txbuf, 1, rxbuf, 6);

  altimeter_data.t_adc = (int32_t)rxbuf[3] << 12 | (int32_t)rxbuf[4] << 4 | (int32_t)rxbuf[5] >> 4;
  altimeter_data.p_adc = (int32_t)rxbuf[0] << 12 | (int32_t)rxbuf[1] << 4 | (int32_t)rxbuf[2] >> 4;

  if (altimeter_data.t_adc != 0x80000 && altimeter_data.p_adc != 0x80000) {
    bmp280_compensate_temperature(&altimeter_data);
    bmp280_compensate_pressure(&altimeter_data);
  }

  i2cReleaseBus(&I2CD1);

  return;
}

THD_WORKING_AREA(waAltimeter, ALTIMETER_THREAD_STACK_SIZE);
THD_FUNCTION(thAltimeter, arg)
{
  (void)arg;

  chRegSetThreadName("thAltimeter");

  altimeter_init();

  while (true) {
    altimeter_read();
    chThdSleepMilliseconds(500);
  }
}

void shellcmd_altimeter(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "%6d%d\r\n",
             0);
    chThdSleepMilliseconds(50);
  }
}
