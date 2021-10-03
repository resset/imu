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

#include "ch.h"
#include "hal.h"
#include "shell.h"

#include "system.h"
#include "controller.h"
#include "imu.h"
#include "gnss.h"
#include "altimeter.h"
#include "ground_control.h"
#include "servo.h"
#include "shellconf.h"
#include "shell_utils.h"
#include "shell_init.h"

SerialConfig serial_cfg = {
  115200,
  0,
  0,
  0
};

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(2048)

static const ShellCommand commands[] = {
  {"about", shellcmd_about},
  {"reset", shellcmd_reset},
  {"date", shellcmd_date},
#ifdef CH_DBG_STATISTICS
  {"cpu", cpu_load},
#endif
  {"ctrl", shellcmd_controller},
  {"imu", shellcmd_imu},
  {"gnss", shellcmd_servo},
  {"alt", shellcmd_altimeter},
  {"gndc", shellcmd_ground_control},
  {"servo", shellcmd_servo},
  {NULL, NULL}
};

#if (SHELL_USE_HISTORY == TRUE)
char commands_buffer[SHELL_MAX_HIST_BUFF];
#endif

static ShellConfig shell_cfg = {
  (BaseSequentialStream *)&SD3,
  commands
#if (SHELL_USE_HISTORY == TRUE)
  ,
  commands_buffer,
  SHELL_MAX_HIST_BUFF
#endif
};

THD_WORKING_AREA(waShell, SHELL_THREAD_STACK_SIZE);
THD_FUNCTION(thShell, arg)
{
  (void)arg;

  thread_t *shelltp = NULL;

  chRegSetThreadName("shell_init");

  /*
   * Initializes a serial driver.
   */
  sdStart(&SD3, &serial_cfg);
  palSetPadMode(GPIOD, 8, PAL_MODE_ALTERNATE(7)); /* TX */
  palSetPadMode(GPIOD, 9, PAL_MODE_ALTERNATE(7)); /* RX */

  /*
   * Shell manager initialization.
   */
  shellInit();

  while (true) {
    if (SD3.state == SD_READY) {
      shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
                                    "shell", NORMALPRIO + 1,
                                    shellThread, &shell_cfg);
      chThdWait(shelltp); /* Waiting termination.*/
    }
    chThdSleepMilliseconds(500);
  }
}
