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

#include "tasks.h"
#include "imu.h"
#include "controller.h"
#include "gnss.h"
#include "altimeter.h"
#include "ground_control.h"
#include "servo.h"
#include "blackbox.h"
#include "shell_init.h"
#include "blink.h"

void tasks_init(void)
{
  chThdCreateStatic(waImu, sizeof(waImu), HIGHPRIO, thImu, NULL);
  chThdCreateStatic(waController, sizeof(waController), HIGHPRIO - 1, thController, NULL);

  chThdCreateStatic(waGnss, sizeof(waGnss), NORMALPRIO, thGnss, NULL);
  //chThdCreateStatic(waAltimeter, sizeof(waAltimeter), NORMALPRIO, thAltimeter, NULL);
  chThdCreateStatic(waGroundControl, sizeof(waGroundControl), NORMALPRIO, thGroundControl, NULL);

  chThdCreateStatic(waServo, sizeof(waServo), NORMALPRIO, thServo, NULL);

  chThdCreateStatic(waBlackbox, sizeof(waBlackbox), LOWPRIO + 3, thBlackbox, NULL);
  chThdCreateStatic(waShell, sizeof(waShell), LOWPRIO + 2, thShell, NULL);
  chThdCreateStatic(waBlink, sizeof(waBlink), LOWPRIO + 1, thBlink, NULL);
}
