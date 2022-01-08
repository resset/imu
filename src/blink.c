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

THD_WORKING_AREA(waBlink, BLINK_THREAD_STACK_SIZE);
THD_FUNCTION(thBlink, arg)
{
  (void)arg;

  chRegSetThreadName("blink");

  palSetPadMode(GPIOD, 10, PAL_MODE_OUTPUT_PUSHPULL);

  while (true) {
    palSetPad(GPIOD, 10);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, 10);
    chThdSleepMilliseconds(500);
  }
}
