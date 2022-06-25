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

#ifndef _RX_H_
#define _RX_H_

#include "ch.h"
#include "hal.h"

typedef struct {
  uint16_t channels[16];
  uint8_t channel17;
  uint8_t channel18;
  uint8_t lost_frame;
  uint8_t failsafe;
} rx_data_t;

extern rx_data_t rx_data;

void rx_sync_init(void);
void rx_copy_data(rx_data_t *source, rx_data_t *target);

#define RX_THREAD_STACK_SIZE 128
extern THD_WORKING_AREA(waGroundControl, RX_THREAD_STACK_SIZE);
THD_FUNCTION(thGroundControl, arg);
void shellcmd_rx(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _RX_H_ */
