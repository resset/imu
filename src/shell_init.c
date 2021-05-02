/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
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

#include "shellconf.h"
#include "shell_init.h"
#include "shell_utils.h"
#include "barometer_shell.h"
#include "gyro_shell.h"

SerialConfig serial_cfg = {
  115200,
  0,
  0,
  0
};

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(2048)

static const ShellCommand commands[] = {
  {"about", cmd_about},
  {"reset", cmd_reset},
  {"baro", cmd_baro},
  {"gyro", cmd_gyro},
  {NULL, NULL}
};

#if (SHELL_USE_HISTORY == TRUE)
char commands_buffer[SHELL_MAX_HIST_BUFF];
#endif

static ShellConfig shell_cfg = {
  (BaseSequentialStream *)&SD2,
  commands
#if (SHELL_USE_HISTORY == TRUE)
  ,
  commands_buffer,
  SHELL_MAX_HIST_BUFF
#endif
};

THD_WORKING_AREA(waShell, 128);
THD_FUNCTION(thShell, arg) {
  thread_t *shelltp = NULL;

  (void)arg;
  chRegSetThreadName("thShell");

  /*
   * Initializes a serial driver.
   */
  sdStart(&SD2, &serial_cfg);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

  /*
   * Shell manager initialization.
   */
  shellInit();

  while (true) {
    if (SD2.state == SD_READY) {
      shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
                                    "shell", NORMALPRIO + 1,
                                    shellThread, &shell_cfg);
      chThdWait(shelltp); /* Waiting termination.*/
    }
    chThdSleepMilliseconds(500);
  }
}
