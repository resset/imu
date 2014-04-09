/*
    IMU - Copyright (C) 2014 Mateusz Tomaszkiewicz

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

#include "bar_main.h"

uint32_t d1; /* Digital pressure value.*/
uint32_t d2; /* Digital temperature value.*/
int32_t dt;
int32_t temp;
int64_t off;
int64_t sens;
int64_t p;

uint16_t c1, c2, c3, c4, c5, c6;

static void bar_init(void) {

  i2c_sensors_init();

  uint8_t txbuf[1], rxbuf[2];

  i2cAcquireBus(&I2CD1);

  txbuf[0] = MS5611_CMD_RESET;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 0, MS2ST(0x3000));

  chThdSleepMilliseconds(10);

  rxbuf[0] = 0;
  rxbuf[1] = 0;

  txbuf[0] = MS5611_CMD_READ_C0;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c1 = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_C1;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c2 = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_C2;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c3 = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_C3;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c4 = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_C4;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c5 = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_C5;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c6 = (rxbuf[0] << 8) | rxbuf[1];

  i2cReleaseBus(&I2CD1);

  return;
}

static void bar_read(void) {
/*  uint32_t d1; // Digital pressure value.
  uint32_t d2; // Digital temperature value.
  int32_t dt;
  int32_t temp;
  int64_t off;
  int64_t sens;
  int64_t p;*/

  uint8_t txbuf[1], rxbuf[3];

  rxbuf[0] = 0;
  rxbuf[1] = 0;
  rxbuf[2] = 0;

  i2cAcquireBus(&I2CD1);

  txbuf[0] = MS5611_CMD_CONVERT_D1_1024;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 0, MS2ST(0x3000));
  chThdSleepMilliseconds(10);
  txbuf[0] = MS5611_CMD_READ_ADC;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 3, MS2ST(0x3000));
  d1 = (rxbuf[0] << 16) | (rxbuf[1] << 8) | rxbuf[2];

  txbuf[0] = MS5611_CMD_CONVERT_D2_1024;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 0, MS2ST(0x3000));
  chThdSleepMilliseconds(10);
  txbuf[0] = MS5611_CMD_READ_ADC;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 3, MS2ST(0x3000));
  d2 = (rxbuf[0] << 16) | (rxbuf[1] << 8) | rxbuf[2];

  i2cReleaseBus(&I2CD1);

  dt = d2 - (c5 << 8);
  temp = 2000 + (((int64_t)dt * (int64_t)c6) >> 23);

  off = ((uint64_t) c2 << 16) + (((int64_t)c4 * (int64_t)dt) >> 7);
  sens = ((uint64_t) c1 << 15) + (((int64_t)c3 * (int64_t)dt) >> 8);
  p = ((((int64_t)d1 * sens) >> 21) - off) / 32768;

  return;
}

WORKING_AREA(waBar, 128);
msg_t thBar(void *arg) {
  (void)arg;
  chRegSetThreadName("thBar");

  bar_init();

  while (TRUE) {
    bar_read();
    chThdSleepMilliseconds(500);
  }

  return (msg_t)0;
}
