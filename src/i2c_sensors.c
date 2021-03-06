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

#include "hal.h"

#include "i2c_sensors.h"

const I2CConfig i2ccfg = {
  STM32_TIMINGR_PRESC(0x0U) |
  STM32_TIMINGR_SCLDEL(0x9U) | STM32_TIMINGR_SDADEL(0x0U) |
  STM32_TIMINGR_SCLH(0x19U) | STM32_TIMINGR_SCLL(0x4BU),
  0,
  0
};

bool i2c_sensors_started = false;

void i2c_sensors_init(void)
{
  if (!i2c_sensors_started) {
    /*
     * I2C initialization.
     */
    /* palSetPadMode(GPIOC, 13, PAL_MODE_INPUT | PAL_STM32_PUPDR_FLOATING);        / * TP IRQ FIXME.*/
    palSetPadMode(GPIOB, 6, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN); /* SCL */
    palSetPadMode(GPIOB, 7, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN); /* SDA */

    i2cStart(&I2CD1, &i2ccfg);

    i2c_sensors_started = true;
  }

  return;
}
