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

#include "barometer.h"
#include "barometer_shell.h"

void cmd_baro(BaseSequentialStream *chp, int argc, char *argv[]) {

  if (argc == 0) {
    goto ERROR;
  }

  if (argc == 1) {
    if (strcmp(argv[0], "get") == 0) {

      chprintf(chp, "c1: %d\r\n", c[1]);
      chprintf(chp, "c2: %d\r\n", c[2]);
      chprintf(chp, "c3: %d\r\n", c[3]);
      chprintf(chp, "c4: %d\r\n", c[4]);
      chprintf(chp, "c5: %d\r\n", c[5]);
      chprintf(chp, "c6: %d\r\n", c[6]);
      chprintf(chp, "d1: %d\r\n", d1);
      chprintf(chp, "d2: %d\r\n\r\n", d2);

      chprintf(chp, "dt: %d\r\n", dt);
      chprintf(chp, "temp: %d\r\n", temp);
      chprintf(chp, "off: %LD\r\n", off);
      chprintf(chp, "sens: %LD\r\n", sens);
      chprintf(chp, "p: %LD\r\n", p);

      return;
    } else if ((argc == 2) && (strcmp(argv[0], "set") == 0)) {
      chprintf(chp, "set\r\n");
      return;
    }
  }

ERROR:
  chprintf(chp, "Usage: baro get\r\n");
  chprintf(chp, "       baro set x\r\n");
  chprintf(chp, "where x is something\r\n");
  chprintf(chp, "and that's it\r\n");
  return;
}
