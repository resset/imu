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

#include "bmp280.h"

typedef struct {
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;
  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;
  int32_t t_adc;
  int32_t t_fine;
  int32_t p_adc;
  int32_t temperature;
  uint32_t pressure;
  int32_t altitude;
} altimeter_data_t;

#define ALTIMETER_THREAD_STACK_SIZE 256

#define BMP280_ADDR BMP280_ADDR_LOW

extern THD_WORKING_AREA(waAltimeter, ALTIMETER_THREAD_STACK_SIZE);
THD_FUNCTION(thAltimeter, arg);

void shellcmd_altimeter(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _ALTIMETER_H_ */
