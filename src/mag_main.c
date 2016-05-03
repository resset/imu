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

#include "mag_main.h"

int mag_tmp = 0;

static void mag_init(void) {
  return;
}

static void mag_read(void) {
  mag_tmp++;

  return;
}

THD_WORKING_AREA(waMag, 128);
THD_FUNCTION(thMag, arg) {
  (void)arg;
  chRegSetThreadName("thMag");

  mag_init();

  while (true) {
    mag_read();
    chThdSleepMilliseconds(542);
  }
}
