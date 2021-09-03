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

#include "blackbox.h"

static binary_semaphore_t blackbox_ready_bsem;

void blackbox_sync_init(void)
{
  chBSemWait(&blackbox_ready_bsem);
}

THD_WORKING_AREA(waBlackbox, 128);
THD_FUNCTION(thBlackbox, arg)
{
  (void)arg;

  chRegSetThreadName("thBlackbox");

  chBSemObjectInit(&blackbox_ready_bsem, true);
  chBSemSignal(&blackbox_ready_bsem);

  while (true) {
    chThdSleepMilliseconds(500);
  }
}
