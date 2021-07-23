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

#ifndef _GNSS_H_
#define _GNSS_H_

#include "ch.h"
#include "hal.h"

extern THD_WORKING_AREA(waGnss, 128);
THD_FUNCTION(thGnss, arg);
extern thread_t *gnss_thread;

void shellcmd_gnss(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _GNSS_H_ */
