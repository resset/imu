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

#ifndef _GROUND_CONTROL_H_
#define _GROUND_CONTROL_H_

#include "ch.h"
#include "hal.h"

typedef struct {
  uint16_t channels[16];
  uint8_t channel17;
  uint8_t channel18;
  uint8_t lost_frame;
  uint8_t failsafe;
} ground_control_data_t;

extern binary_semaphore_t ground_control_ready_bsem;
extern mutex_t ground_control_data_mtx;
extern ground_control_data_t ground_control_data;

void ground_control_copy_data(ground_control_data_t *source, ground_control_data_t *target);

extern THD_WORKING_AREA(waGroundControl, 128);
THD_FUNCTION(thGroundControl, arg);

void shellcmd_ground_control(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _GROUND_CONTROL_H_ */
