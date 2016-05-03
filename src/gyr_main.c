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

#include "gyr_main.h"

int gyr_tmp = 0;

static void gyr_init(void) {
  return;
}

static void gyr_read(void) {
  gyr_tmp++;

  return;
}

WORKING_AREA(waGyr, 128);
msg_t thGyr(void *arg) {
  (void)arg;
  chRegSetThreadName("thGyr");

  gyr_init();

  while (TRUE) {
    gyr_read();
    chThdSleepMilliseconds(321);
  }

  return (msg_t)0;
}
