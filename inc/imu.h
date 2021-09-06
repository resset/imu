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

#ifndef _IMU_H_
#define _IMU_H_

#include "ch.h"
#include "hal.h"

typedef struct {
  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;
  int16_t temperature;
} imu_data_t;

extern imu_data_t imu_data;

void imu_sync_init(void);

extern THD_WORKING_AREA(waImu, 128);
THD_FUNCTION(thImu, arg);

void shellcmd_imu(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _IMU_H_ */
