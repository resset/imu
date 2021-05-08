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

#include "ch.h"
#include "hal.h"

#include "blink.h"

THD_WORKING_AREA(waBlink, 128);
THD_FUNCTION(thBlink, arg)
{
  (void)arg;

  chRegSetThreadName("thBlink");

  while (true) {
    palClearPad(GPIOD, GPIOD_LED4);
    palSetPad(GPIOD, GPIOD_LED3);
    palSetPad(GPIOD, GPIOD_LED5);
    palSetPad(GPIOD, GPIOD_LED6);
    chThdSleepMilliseconds(300);
    palSetPad(GPIOD, GPIOD_LED4);
    palClearPad(GPIOD, GPIOD_LED3);
    palSetPad(GPIOD, GPIOD_LED5);
    palSetPad(GPIOD, GPIOD_LED6);
    chThdSleepMilliseconds(300);
    palSetPad(GPIOD, GPIOD_LED4);
    palSetPad(GPIOD, GPIOD_LED3);
    palClearPad(GPIOD, GPIOD_LED5);
    palSetPad(GPIOD, GPIOD_LED6);
    chThdSleepMilliseconds(300);
    palSetPad(GPIOD, GPIOD_LED4);
    palSetPad(GPIOD, GPIOD_LED3);
    palSetPad(GPIOD, GPIOD_LED5);
    palClearPad(GPIOD, GPIOD_LED6);
    chThdSleepMilliseconds(300);
  }
}
