/*
    IMU - Copyright (C) 2014-2016 Mateusz Tomaszkiewicz

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

#include "blink_main.h"

static const PWMConfig pwmcfg = {
  100000,
  128,
  NULL,
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},
   {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  },
  0,
  0
};

THD_WORKING_AREA(waBlink, 128);
THD_FUNCTION(thBlink, arg) {
  uint8_t i = 0;

  (void)arg;
  chRegSetThreadName("thBlink");

  pwmStart(&PWMD4, &pwmcfg);
  palSetPadMode(GPIOD, GPIOD_LED4, PAL_MODE_ALTERNATE(2));        /* Green. */
  palSetPadMode(GPIOD, GPIOD_LED3, PAL_MODE_ALTERNATE(2));        /* Orange.*/
  palSetPadMode(GPIOD, GPIOD_LED5, PAL_MODE_ALTERNATE(2));        /* Red.   */
  palSetPadMode(GPIOD, GPIOD_LED6, PAL_MODE_ALTERNATE(2));        /* Blue.  */

  while (true) {
    pwmEnableChannel(&PWMD4, 0, (pwmcnt_t)i);
    pwmEnableChannel(&PWMD4, 1, (pwmcnt_t)(i + 64));
    pwmEnableChannel(&PWMD4, 2, (pwmcnt_t)(i + 128));
    pwmEnableChannel(&PWMD4, 3, (pwmcnt_t)(i + 192));

    i++;

    chThdSleepMilliseconds(50);
  }
}
