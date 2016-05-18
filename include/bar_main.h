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

#ifndef _BAR_MAIN_H_
#define _BAR_MAIN_H_

#include "ch.h"
#include "hal.h"

#include "i2c_sensors.h"
#include "ms5611.h"

#define MS5611_I2C_ADDR MS5611_I2C_ADDR_LOW

#define BAR_THREAD_STACK_SIZE 256

extern uint16_t c[8];
extern uint32_t d1;
extern uint32_t d2;
extern int32_t dt;
extern int64_t temp;
extern int64_t off;
extern int64_t sens;
extern int64_t p;

extern THD_WORKING_AREA(waBar, BAR_THREAD_STACK_SIZE);
THD_FUNCTION(thBar, arg);

#endif /* _BAR_MAIN_H_ */
