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

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "pg.h"
#include "display.h"

static binary_semaphore_t display_ready_bsem;

static const I2CConfig i2ccfg = {
  STM32_TIMINGR_PRESC(0x0U) |
  STM32_TIMINGR_SCLDEL(0x9U) | STM32_TIMINGR_SDADEL(0x0U) |
  STM32_TIMINGR_SCLH(0x19U) | STM32_TIMINGR_SCLL(0x4BU),
  0,
  0
};

static void i2c_transmit(uint8_t *txbuf, size_t txbuf_len, uint8_t *rxbuf, size_t rxbuf_len)
{
  cacheBufferFlush(txbuf, CACHE_SIZE_ALIGN(uint8_t, txbuf_len));
  cacheBufferInvalidate(rxbuf, CACHE_SIZE_ALIGN(uint8_t, rxbuf_len));
  i2cMasterTransmitTimeout(&I2CD2, SH1106_ADDR, txbuf, txbuf_len, rxbuf, rxbuf_len, TIME_INFINITE);
}

static void i2c_send(uint8_t *txbuf, size_t txbuf_len)
{
  cacheBufferFlush(txbuf, CACHE_SIZE_ALIGN(uint8_t, txbuf_len));
  i2cMasterTransmitTimeout(&I2CD2, SH1106_ADDR, txbuf, txbuf_len, NULL, 0, TIME_INFINITE);
}

static pg_result_t display_init(void)
{
  CC_ALIGN_DATA(32) uint8_t txbuf[CACHE_SIZE_ALIGN(uint8_t, 2)];
  CC_ALIGN_DATA(32) uint8_t rxbuf[CACHE_SIZE_ALIGN(uint8_t, 24)];

  /*
   * I2C initialization.
   */
  palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN); /* SCL */
  palSetPadMode(GPIOB, 11, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN); /* SDA */

  i2cStart(&I2CD2, &i2ccfg);

  i2cAcquireBus(&I2CD2);

  /* Test for SH1106.*/
  txbuf[0] = SH1106_ID;
  i2c_transmit(txbuf, 1, rxbuf, 1);

  if (rxbuf[0] != SH1106_VAL_ID) {
    i2cReleaseBus(&I2CD2);
    return PG_ERROR;
  }

  i2cReleaseBus(&I2CD2);

  return PG_OK;
}

void display_sync_init(void)
{
  chBSemWait(&display_ready_bsem);
}

THD_WORKING_AREA(waDisplay, DISPLAY_THREAD_STACK_SIZE);
THD_FUNCTION(thDisplay, arg)
{
  (void)arg;

  chRegSetThreadName("display");

  display_init();

  chBSemObjectInit(&display_ready_bsem, true);
  chBSemSignal(&display_ready_bsem);

  while (true) {
    chThdSleepMilliseconds(50);
  }
}

void shellcmd_display(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  /* Currently we are allowed to check once, because READY state is a dead-end.*/
  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "display\r\n");
    chThdSleepMilliseconds(100);
  }
}
