/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
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
#include "chprintf.h"

#include "shell_utils.h"

void shellcmd_about(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;

  /* TODO: add information. */
  chprintf(chp, "TODO\r\n");
}

void shellcmd_reset(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;

  chprintf(chp, "Resetting...\r\n");
  /* TODO: an actual reset. */
  chThdSleepMilliseconds(10);
  SCB->AIRCR = 0x05FA << SCB_AIRCR_VECTKEY_Pos | 1 << SCB_AIRCR_SYSRESETREQ_Pos;
  while (1);
}
