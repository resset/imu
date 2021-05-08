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

#ifndef _GYRO_H_
#define _GYRO_H_

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "i2c_sensors.h"

extern int gyr_tmp;

extern THD_WORKING_AREA(waGyr, 128);
THD_FUNCTION(thGyr, arg);

void shellcmd_gyro(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _GYRO_H_ */
