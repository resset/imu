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

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "gyro.h"

int gyr_tmp = 0;

static void gyr_init(void)
{
  return;
}

static void gyr_read(void)
{
  return;
}

THD_WORKING_AREA(waGyr, 128);
THD_FUNCTION(thGyr, arg)
{
  (void)arg;

  chRegSetThreadName("thGyr");

  gyr_init();

  while (true) {
    gyr_read();
    chThdSleepMilliseconds(321);
  }
}

void shellcmd_gyro(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc == 0) {
    goto ERROR;
  }

  if (argc == 1) {
    if (strcmp(argv[0], "get") == 0) {
      chprintf(chp, "got %d\r\n", 777);
      return;
    } else if ((argc == 2) && (strcmp(argv[0], "set") == 0)) {
      chprintf(chp, "set\r\n");
      return;
    }
  }

ERROR:
  chprintf(chp, "Usage: gyro get\r\n");
  chprintf(chp, "       gyro set x\r\n");
  chprintf(chp, "where x is something\r\n");
  chprintf(chp, "and that's it\r\n");
  return;
}
