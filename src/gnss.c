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

#include "gnss.h"

static thread_t *gnss_thread = NULL;
static binary_semaphore_t gnss_ready_bsem;

/* GNSS specific SIO configuration. GNSS serial parameters are:
 * - baud rate of 9600
 * - 8 data bits
 * - no parity
 * - 1 stop bit
 */
static SIOConfig sio8_config = {
  9600,
  USART_PRESC1,
  USART_CR1_FIFOEN,
  0,
  USART_CR3_RXFTCFG_7E
};

static void rxfifo(SIODriver *siop)
{
  (void)siop;
  chSysLockFromISR();
  chEvtSignalI(gnss_thread, EVENT_MASK(1));
  chSysUnlockFromISR();
}

static void rxidle(SIODriver *siop)
{
  (void)siop;
  chSysLockFromISR();
  chEvtSignalI(gnss_thread, EVENT_MASK(0));
  chSysUnlockFromISR();
}

static SIOOperation sio8_operation = {
  .rx_cb      = rxfifo,
  .rx_idle_cb = rxidle,
  .tx_cb      = NULL,
  .tx_end_cb  = NULL,
  .rx_evt_cb  = NULL
};

void gnss_sync_init(void)
{
  chBSemWait(&gnss_ready_bsem);
}

THD_WORKING_AREA(waGnss, GNSS_THREAD_STACK_SIZE);
THD_FUNCTION(thGnss, arg)
{
  (void)arg;

  eventmask_t evt;

  chRegSetThreadName("gnss");
  gnss_thread = chThdGetSelfX();
  chBSemObjectInit(&gnss_ready_bsem, true);

  sioStart(&SIOD8, &sio8_config);
  sioStartOperation(&SIOD8, &sio8_operation);
  palSetPadMode(GPIOE, 1, PAL_MODE_ALTERNATE(8)); /* TX */
  palSetPadMode(GPIOE, 0, PAL_MODE_ALTERNATE(8)); /* RX */

  chBSemSignal(&gnss_ready_bsem);

  /* This loop is suited to GNSS transmission timing characteristics.
     Its event processing expect GNSS_PACKET_LENGTH bytes in a sequence,
     then an idle period on which it triggers packet processing. First idle
     event is used for synchronization.*/
  while (true) {
    evt = chEvtWaitAny(ALL_EVENTS);

    /* IDLE event, get rest of the data from FIFO and process packet.*/
    if (evt & EVENT_MASK(0)) {
    }

    /* Copy FIFO data when FIFO threshold reached.*/
    if (evt & EVENT_MASK(1)) {
    }

    chThdSleepMilliseconds(500);
  }
}

void shellcmd_gnss(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chThdSleepMilliseconds(50);
  }

  return;
}
