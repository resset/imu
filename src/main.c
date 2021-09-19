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

#include "system.h"
#include "tasks.h"

int main(void)
{
  /* HAL and RTOS initialization.*/
  halInit();
  chSysInit();

  /* Common functions start.*/
  system_init();

  /* Change main() thread priority to the lowest value. This thread
     has no specific function.*/
  chThdSetPriority(LOWPRIO);

  /* Create and start threads. From now on, all code execution
     is governed there.*/
  tasks_init();

  /* Exit startup thread, we do not need it anymore.*/
  chThdExit((msg_t)0);

  return 0;
}
