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

#ifndef _ALTIMETER_H_
#define _ALTIMETER_H_

#include "ch.h"
#include "hal.h"

#include "pg.h"
#include "bmp280.h"

#define BMP280_ADDR BMP280_ADDR_LOW

#define ALTIMETER_THREAD_STACK_SIZE 256

extern THD_WORKING_AREA(waAltimeter, ALTIMETER_THREAD_STACK_SIZE);
THD_FUNCTION(thAltimeter, arg);

pg_result_t altimeter_state_zero(void);

void shellcmd_altimeter(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _ALTIMETER_H_ */
