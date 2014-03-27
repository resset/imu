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

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "main.h"
#include "blink_main.h"
#include "shell_main.h"

Thread *tpBlink;
Thread *tpShell;

int main(void) {
  halInit();
  chSysInit();

  tpBlink = chThdCreateStatic(waBlink, sizeof(waBlink),
                    NORMALPRIO, thBlink, NULL);
  tpShell = chThdCreateStatic(waShell, sizeof(waShell),
                    NORMALPRIO, thShell, NULL);

  while (TRUE) {
    chThdSleepMilliseconds(500);
  }

  return 0;
}
