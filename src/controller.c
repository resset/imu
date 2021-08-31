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

#include "controller.h"
#include "altimeter.h"

typedef enum {
  CONTROLLER_STATE_INIT,
  CONTROLLER_STATE_WAIT,
  CONTROLLER_STATE_READY,
  CONTROLLER_FATAL_ERROR
} controller_state_t;

static controller_state_t controller_state;

static void controller_loop(void)
{
  /* Gather sensor data.*/
  /* Process data.*/
  /* Output control signals.*/
}

THD_WORKING_AREA(waController, 128);
THD_FUNCTION(thController, arg)
{
  (void)arg;

  chRegSetThreadName("thController");

  controller_state = CONTROLLER_STATE_INIT;
  controller_state = CONTROLLER_STATE_WAIT;

  chBSemWait(&altimeter_ready_bsem);

  controller_state = CONTROLLER_STATE_READY;

  systime_t time = chVTGetSystemTime();
  while (true) {
    /* Fire the thread every 1 ms (1 kHz).*/
    time = chTimeAddX(time, TIME_MS2I(1));

    controller_loop();

    chThdSleepUntil(time);
  }
}

void shellcmd_controller(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  chprintf(chp, "controller state: %d\r\n", (int32_t)controller_state);
}
