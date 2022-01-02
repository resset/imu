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

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "ch.h"
#include "hal.h"

#include "pg.h"

#define SH1106_ID 0x00
#define SH1106_VAL_ID 0x00

#define SH1106_ADDR_ADDR_LOW 0x3C
#define SH1106_ADDR_ADDR_HIGH 0x3D

#define SH1106_ADDR SH1106_ADDR_ADDR_LOW

void display_sync_init(void);

#define DISPLAY_THREAD_STACK_SIZE 128
extern THD_WORKING_AREA(waDisplay, DISPLAY_THREAD_STACK_SIZE);
THD_FUNCTION(thDisplay, arg);
void shellcmd_display(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _DISPLAY_H_ */
