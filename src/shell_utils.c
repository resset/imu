/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio
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

#include "shell_utils.h"

void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, total, largest;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = chHeapStatus(NULL, &total, &largest);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreGetStatusX());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", total);
  chprintf(chp, "heap free largest: %u bytes\r\n", largest);
}

void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "           name     addr    stack prio refs     state\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%15s %08lx %08lx %4lu %4lu %9s\r\n",
             (uint32_t)tp->name, (uint32_t)tp, (uint32_t)tp->ctx.sp,
             (uint32_t)tp->prio, (uint32_t)(tp->refs - 1),
             states[tp->state]);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}
