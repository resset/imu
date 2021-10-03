/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
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

#include <stdlib.h>

#include "hal.h"
#include "chprintf.h"

#include "shell_utils.h"

void shellcmd_about(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;

  /* TODO: add information. */
  chprintf(chp, "\r\n");
}

void shellcmd_reset(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;

  chprintf(chp, "Resetting...\r\n");
  chThdSleepMilliseconds(10);

  SCB->AIRCR = 0x05FA << SCB_AIRCR_VECTKEY_Pos | 1 << SCB_AIRCR_SYSRESETREQ_Pos;
  while (1);
}

static void print_time(BaseSequentialStream *chp, RTCDateTime *timespec)
{
  chprintf(chp, "%04d-%02d-%02d %02d:%02d:%02d.%03d\r\n",
          timespec->year + 1980U,
          timespec->month,
          timespec->day,
          timespec->millisecond / 3600000U,
          (timespec->millisecond % 3600000U) / 60000U,
          (timespec->millisecond % 60000U) / 1000U,
          timespec->millisecond % 1000U
          );
}

void shellcmd_date(BaseSequentialStream *chp, int argc, char *argv[])
{
  RTCDateTime timespec;
  time_t unix_time;
  struct tm tim;
  struct tm *canary;

  if (argc == 0) {
    rtcGetTime(&RTCD1, &timespec);
    print_time(chp, &timespec);
  } else if (argc == 1) {
    unix_time = atol(argv[0]);
    canary = localtime_r(&unix_time, &tim);
    if (&tim == canary) {
      rtcConvertStructTmToDateTime(&tim, 0, &timespec);
      rtcSetTime(&RTCD1, &timespec);
      rtcGetTime(&RTCD1, &timespec);
      print_time(chp, &timespec);
    } else {
      chprintf(chp, "Error: time setting failed.\r\n");
    }
  } else {
    chprintf(chp, "Usage: date [timestamp]\r\n");
  }
}
