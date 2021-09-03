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

#include "ground_control.h"
#include "servo.h"

#define SBUS_PACKET_LENGTH     25
#define SBUS_HEADER          0x0f
#define SBUS_FOOTER          0x00
#define SBUS_CH17_MASK       0x01
#define SBUS_CH18_MASK       0x02
#define SBUS_LOST_FRAME_MASK 0x04
#define SBUS_FAILSAFE_MASK   0x08

#define SIO_FIFO_LENGTH        16

#define EVT_RESET EVENT_MASK(0)
#define EVT_DATA  EVENT_MASK(1)

binary_semaphore_t ground_control_ready_bsem;
mutex_t ground_control_data_mtx;
ground_control_data_t ground_control_data, gcd;

static thread_t *ground_control_thread = NULL;
binary_semaphore_t ground_control_ready_bsem;

static uint8_t rxbuffer[SIO_FIFO_LENGTH];
static uint8_t buffer[SBUS_PACKET_LENGTH];

/*
 * This algorithm is meant to deliver frames as quick as possible.
 * There is no fancy filtering. If one packet is too short or has incorrect
 * footer we will miss two packets. If it is too long or has incorrect header,
 * we will miss only this packet.
 */
uint8_t sbus_decode_packet(uint8_t current_byte)
{
  static uint8_t packet_length = 0;
  static uint8_t previous_byte = SBUS_FOOTER;

  if (packet_length == 0) {
    if (current_byte == SBUS_HEADER && previous_byte == SBUS_FOOTER) {
      buffer[packet_length] = current_byte;
      packet_length++;
    }
  } else {
    buffer[packet_length] = current_byte;
    if (packet_length < SBUS_PACKET_LENGTH - 1) {
      packet_length++;
    } else {
      packet_length = 0;
      if (current_byte == SBUS_FOOTER) {
        return 1;
      }
    }
  }
  previous_byte = current_byte;
  return 0;
}

/* SBUS specific SIO configuration. Since SBUS is an inverted protocol,
 * we enable RX pin active level inversion. SBUS serial parameters are:
 * - baud rate of 100000
 * - 8 data bits
 * - even parity
 * - 2 stop bits
 */
static SIOConfig sio2_config = {
  100000,
  USART_PRESC1,
  USART_CR1_FIFOEN | USART_CR1_M0 | USART_CR1_PCE, /* M[1:0] = '01' means 9 bits to store parity
                                                      bit. PCE bit set and PS bit cleared means
                                                      even parity.*/
  USART_CR2_RXINV | USART_CR2_STOP_1,              /* RX inversion and 2 stop bits.*/
  USART_CR3_RXFTCFG_7E                             /* Interrupt if FIFO reaches 7/8 depth.*/
};

static void rxfifo(SIODriver *siop)
{
  (void)siop;
  chSysLockFromISR();
  chEvtSignalI(ground_control_thread, EVT_DATA);
  chSysUnlockFromISR();
}

static void rxidle(SIODriver *siop)
{
  (void)siop;
  chSysLockFromISR();
  chEvtSignalI(ground_control_thread, EVT_RESET);
  chSysUnlockFromISR();
}

static SIOOperation sio2_operation = {
  .rx_cb      = rxfifo,
  .rx_idle_cb = rxidle,
  .tx_cb      = NULL,
  .tx_end_cb  = NULL,
  .rx_evt_cb  = NULL
};

void sbus_parse_packet(uint8_t *packet)
{
  if (packet[0] == SBUS_HEADER && packet[24] == SBUS_FOOTER) {
    gcd.channels[0]  = packet[1]       | ((uint16_t)packet[2]  << 8  & 0x07FF);
    gcd.channels[1]  = packet[2]  >> 3 | ((uint16_t)packet[3]  << 5  & 0x07FF);
    gcd.channels[2]  = packet[3]  >> 6 |  (uint16_t)packet[4]  << 2
                                              | ((uint16_t)packet[5]  << 10 & 0x07FF);
    gcd.channels[3]  = packet[5]  >> 1 | ((uint16_t)packet[6]  << 7  & 0x07FF);
    gcd.channels[4]  = packet[6]  >> 4 | ((uint16_t)packet[7]  << 4  & 0x07FF);
    gcd.channels[5]  = packet[7]  >> 7 |  (uint16_t)packet[8]  << 1
                                              | ((uint16_t)packet[9]  << 9  & 0x07FF);
    gcd.channels[6]  = packet[9]  >> 2 | ((uint16_t)packet[10] << 6  & 0x07FF);
    gcd.channels[7]  = packet[10] >> 5 | ((uint16_t)packet[11] << 3  & 0x07FF);
    gcd.channels[8]  = packet[12]      | ((uint16_t)packet[13] << 8  & 0x07FF);
    gcd.channels[9]  = packet[13] >> 3 | ((uint16_t)packet[14] << 5  & 0x07FF);
    gcd.channels[10] = packet[14] >> 6 |  (uint16_t)packet[15] << 2
                                              | ((uint16_t)packet[16] << 10 & 0x07FF);
    gcd.channels[11] = packet[16] >> 1 | ((uint16_t)packet[17] << 7  & 0x07FF);
    gcd.channels[12] = packet[17] >> 4 | ((uint16_t)packet[18] << 4  & 0x07FF);
    gcd.channels[13] = packet[18] >> 7 |  (uint16_t)packet[19] << 1
                                              | ((uint16_t)packet[20] << 9  & 0x07FF);
    gcd.channels[14] = packet[20] >> 2 | ((uint16_t)packet[21] << 6  & 0x07FF);
    gcd.channels[15] = packet[21] >> 5 | ((uint16_t)packet[22] << 3  & 0x07FF);

    gcd.channel17 = packet[23] & SBUS_CH17_MASK ? 1 : 0;
    gcd.channel18 = packet[23] & SBUS_CH18_MASK ? 1 : 0;
    gcd.lost_frame = packet[23] & SBUS_LOST_FRAME_MASK ? 1 : 0;
    gcd.failsafe = packet[23] & SBUS_FAILSAFE_MASK ? 1 : 0;
  }
}

void ground_control_copy_data(ground_control_data_t *source, ground_control_data_t *target)
{
  chMtxLock(&ground_control_data_mtx);
  memcpy(target, source, sizeof(ground_control_data_t));
  chMtxUnlock(&ground_control_data_mtx);
}

THD_WORKING_AREA(waGroundControl, 128);
THD_FUNCTION(thGroundControl, arg)
{
  (void)arg;

  eventmask_t evt;
  size_t pos = 0;
  size_t n;

  chRegSetThreadName("thGroundControl");
  ground_control_thread = chThdGetSelfX();
  chBSemObjectInit(&ground_control_ready_bsem, true);
  chMtxObjectInit(&ground_control_data_mtx);

  sioStart(&SIOD2, &sio2_config);
  sioStartOperation(&SIOD2, &sio2_operation);
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7)); /* RX */

  chBSemSignal(&ground_control_ready_bsem);

  /* This loop is suited to SBUS transmission timing characteristics.
     Its event processing expect SBUS_PACKET_LENGTH bytes in a sequence,
     then an idle period on which it triggers packet processing. First idle
     event is used for synchronization.*/
  while (true) {
    evt = chEvtWaitAny(ALL_EVENTS);

    /* IDLE event, get rest of the data from FIFO and process packet.*/
    if (evt & EVT_RESET) {
      if (pos < SBUS_PACKET_LENGTH) {
        n = sioAsyncRead(&SIOD2, rxbuffer, SIO_FIFO_LENGTH);
        if (pos + n == SBUS_PACKET_LENGTH) {
          memcpy(buffer + pos, rxbuffer, n);
          pos += n;
        }
      }

      /* We should have the packet now.*/
      if (pos == SBUS_PACKET_LENGTH) {
        sbus_parse_packet(buffer);
        ground_control_copy_data(&gcd, &ground_control_data);
      }

      /* We received RX IDLE, this means we have to start over.*/
      pos = 0;
    }

    /* Copy FIFO data when FIFO threshold reached.*/
    if (evt & EVT_DATA) {
      n = sioAsyncRead(&SIOD2, rxbuffer, SIO_FIFO_LENGTH);
      if (n && (pos + n) <= SBUS_PACKET_LENGTH) {
        memcpy(buffer + pos, rxbuffer, n);
        pos += n;
      }
    }
  }
}

void shellcmd_ground_control(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d ",
             gcd.channels[0], gcd.channels[1],
             gcd.channels[2], gcd.channels[3],
             gcd.channels[4], gcd.channels[5],
             gcd.channels[6], gcd.channels[7],
             gcd.channels[8], gcd.channels[9],
             gcd.channels[10], gcd.channels[11],
             gcd.channels[12], gcd.channels[13],
             gcd.channels[14], gcd.channels[15]);
    chprintf(chp, "ch17: %d ch18: %d lost_frame: %d failsafe: %d\r\n",
             gcd.channel17, gcd.channel18,
             gcd.lost_frame, gcd.failsafe);
    chThdSleepMilliseconds(50);
  }

  return;
}
