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

#define BMP280_ADDR           BMP280_ADDR_LOW
#define BMP280_ADC_T_MIN      ((int32_t)0x00000)
#define BMP280_ADC_T_MAX      ((int32_t)0xFFFF0)
#define BMP280_ADC_P_MIN      ((int32_t)0x00000)
#define BMP280_ADC_P_MAX      ((int32_t)0xFFFF0)
#define BMP280_MIN_TEMP_INT   ((int32_t)-4000)
#define BMP280_MAX_TEMP_INT   ((int32_t)8500)
#define BMP280_MIN_PRES_64INT ((uint32_t)(30000 * 256))
#define BMP280_MAX_PRES_64INT ((uint32_t)(110000 * 256))

#ifndef PG_CFG_ALT_ZERO_SAMPLES
#define PG_CFG_ALT_ZERO_SAMPLES ((uint16_t)10)
#endif

#ifndef PG_CFG_ALT_ZERO_MAX_TRIES
#define PG_CFG_ALT_ZERO_MAX_TRIES ((uint16_t)100)
#endif

typedef struct {
  bool data_valid;
  float temperature;
  float pressure;
  float pressure_reference;
  float altitude;
} altimeter_data_t;

extern binary_semaphore_t altimeter_ready_bsem;
extern mutex_t altimeter_data_mtx;
extern altimeter_data_t altimeter_data;

pg_result_t altimeter_state_zero(void);
void altimeter_copy_data(altimeter_data_t *source, altimeter_data_t *target);

#define ALTIMETER_THREAD_STACK_SIZE 256
extern THD_WORKING_AREA(waAltimeter, ALTIMETER_THREAD_STACK_SIZE);
THD_FUNCTION(thAltimeter, arg);

void shellcmd_altimeter(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _ALTIMETER_H_ */
