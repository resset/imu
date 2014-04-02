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

uint8_t bar_val = 0;

uint16_t coefficients[6];

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
  coefficients[0] = (rxbuf[1] << 8) | rxbuf[0];

  txbuf[0] = MS5611_CMD_READ_C1;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  coefficients[1] = (rxbuf[1] << 8) | rxbuf[0];

  txbuf[0] = MS5611_CMD_READ_C2;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  coefficients[2] = (rxbuf[1] << 8) | rxbuf[0];

  txbuf[0] = MS5611_CMD_READ_C3;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  coefficients[3] = (rxbuf[1] << 8) | rxbuf[0];

  txbuf[0] = MS5611_CMD_READ_C4;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  coefficients[4] = (rxbuf[1] << 8) | rxbuf[0];

  txbuf[0] = MS5611_CMD_READ_C5;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  coefficients[5] = (rxbuf[1] << 8) | rxbuf[0];

  i2cReleaseBus(&I2CD1);

  return;
}

static void bar_read(void) {
/*  uint8_t txbuf[1], rxbuf[2];

  rxbuf[0] = 0;
  rxbuf[1] = 0;

  i2cAcquireBus(&I2CD1);
  txbuf[0] = 0x75;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 1, MS2ST(0x3000));
  rxbuf[0] = 0;
  txbuf[0] = 0x6b;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 1, MS2ST(0x3000));
  i2cReleaseBus(&I2CD1);*/

  bar_val++;

  return;
}

WORKING_AREA(waBar, 128);
msg_t thBar(void *arg) {
  (void)arg;
  chRegSetThreadName("thBar");

  bar_init();

  while (TRUE) {
    bar_read();
    chThdSleepMilliseconds(100);
  }

  return (msg_t)0;
}
