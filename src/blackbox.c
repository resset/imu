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

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "ff.h"

#include "blackbox.h"

static binary_semaphore_t blackbox_ready_bsem;

void blackbox_sync_init(void)
{
  chBSemWait(&blackbox_ready_bsem);
}

void blackbox_init(void)
{
  sdcStart(&SDCD1, NULL);

  if (sdcConnect(&SDCD1)) {
    return;
  }

  err = f_mount(&SDC_FS, "/", 1);
  if (err != FR_OK) {
    sdcDisconnect(&SDCD1);
    return;
  }
  fs_ready = TRUE;
}

/*===========================================================================*/
/* FatFs related.                                                            */
/*===========================================================================*/

/**
 * @brief FS object.
 */
static FATFS SDC_FS;

/* FS mounted and ready.*/
static bool fs_ready = FALSE;

/* Generic large buffer.*/
static uint8_t fbuff[1024];

static FRESULT scan_files(BaseSequentialStream *chp, char *path) {
  static FILINFO fno;
  FRESULT res;
  DIR dir;
  size_t i;
  char *fn;

  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    i = strlen(path);
    while (((res = f_readdir(&dir, &fno)) == FR_OK) && fno.fname[0]) {
      if (FF_FS_RPATH && fno.fname[0] == '.')
        continue;
      fn = fno.fname;
      if (fno.fattrib & AM_DIR) {
        *(path + i) = '/';
        strcpy(path + i + 1, fn);
        res = scan_files(chp, path);
        *(path + i) = '\0';
        if (res != FR_OK)
          break;
      }
      else {
        chprintf(chp, "%s/%s\r\n", path, fn);
      }
    }
  }
  return res;
}

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)

void shellcmd_tree(BaseSequentialStream *chp, int argc, char *argv[]) {
  FRESULT err;
  uint32_t fre_clust;
  FATFS *fsp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: tree\r\n");
    return;
  }

  if (!fs_ready) {
    chprintf(chp, "File System not mounted\r\n");
    return;
  }

  err = f_getfree("/", &fre_clust, &fsp);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_getfree() failed\r\n");
    return;
  }
  chprintf(chp,
           "FS: %lu free clusters with %lu sectors (%lu bytes) per cluster\r\n",
           fre_clust, (uint32_t)fsp->csize, (uint32_t)fsp->csize * 512);
  fbuff[0] = 0;
  scan_files(chp, (char *)fbuff);
}

static void shellcmd_create(BaseSequentialStream *chp, int argc, char *argv[]) {
  FRESULT err;
  FIL f;
  static const char data[] = "the quick brown fox jumps over the lazy dog";
  UINT btw = sizeof data - 1;
  UINT bw;

  if (argc != 1) {
    chprintf(chp, "Usage: create <filename>\r\n");
    return;
  }

  if (!fs_ready) {
    chprintf(chp, "File System not mounted\r\n");
    return;
  }

  err = f_open(&f, (const TCHAR *)argv[0], FA_CREATE_ALWAYS | FA_WRITE);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_open() failed\r\n");
    return;
  }

  err = f_write(&f, (const void *)data, btw, &bw);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_write() failed\r\n");
  }

  err = f_close(&f);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_close() failed\r\n");
    return;
  }
}

THD_WORKING_AREA(waBlackbox, BLACKBOX_THREAD_STACK_SIZE);
THD_FUNCTION(thBlackbox, arg)
{
  (void)arg;

  chRegSetThreadName("blackbox");

  balckbox_init();

  chBSemObjectInit(&blackbox_ready_bsem, true);
  chBSemSignal(&blackbox_ready_bsem);

  while (true) {
    chThdSleepMilliseconds(500);
  }
}
