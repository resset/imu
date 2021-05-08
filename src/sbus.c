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

#include "sbus.h"
#include "servo.h"

#define SBUS_PACKET_LENGTH     25
#define SBUS_HEADER          0x0f
#define SBUS_FOOTER          0x00
#define SBUS_CH17_MASK       0x01
#define SBUS_CH18_MASK       0x02
#define SBUS_LOST_FRAME_MASK 0x04
#define SBUS_FAILSAFE_MASK   0x08

uint8_t buffer[SBUS_PACKET_LENGTH];

typedef struct {
  uint16_t channels[16];
  uint8_t channel17;
  uint8_t channel18;
  uint8_t lost_frame;
  uint8_t failsafe;
} sbus_state_t;

sbus_state_t sbus_state;

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

/*
 * This callback is invoked on a receive error, the errors mask is passed
 * as parameter.
 */
static void rxerr(UARTDriver *uartp, uartflags_t e) {

  (void)uartp;
  (void)e;
}

/*
 * This callback is invoked when a character is received but the application
 * was not ready to receive it, the character is passed as parameter.
 */
static void rxchar(UARTDriver *uartp, uint16_t c) {

  (void)uartp;

  char a[1] = {(char)c};
  uartStartSend(&UARTD6, 1, a);

  if (sbus_decode_packet((uint8_t)c)) {
    sbus_state.channels[0]  = buffer[1]       | ((uint16_t)buffer[2]  << 8  & 0x07FF);
    sbus_state.channels[1]  = buffer[2]  >> 3 | ((uint16_t)buffer[3]  << 5  & 0x07FF);
    sbus_state.channels[2]  = buffer[3]  >> 6 |  (uint16_t)buffer[4]  << 2
                                              | ((uint16_t)buffer[5]  << 10 & 0x07FF);
    sbus_state.channels[3]  = buffer[5]  >> 1 | ((uint16_t)buffer[6]  << 7  & 0x07FF);
    sbus_state.channels[4]  = buffer[6]  >> 4 | ((uint16_t)buffer[7]  << 4  & 0x07FF);
    sbus_state.channels[5]  = buffer[7]  >> 7 |  (uint16_t)buffer[8]  << 1
                                              | ((uint16_t)buffer[9]  << 9  & 0x07FF);
    sbus_state.channels[6]  = buffer[9]  >> 2 | ((uint16_t)buffer[10] << 6  & 0x07FF);
    sbus_state.channels[7]  = buffer[10] >> 5 | ((uint16_t)buffer[11] << 3  & 0x07FF);
    sbus_state.channels[8]  = buffer[12]      | ((uint16_t)buffer[13] << 8  & 0x07FF);
    sbus_state.channels[9]  = buffer[13] >> 3 | ((uint16_t)buffer[14] << 5  & 0x07FF);
    sbus_state.channels[10] = buffer[14] >> 6 |  (uint16_t)buffer[15] << 2
                                              | ((uint16_t)buffer[16] << 10 & 0x07FF);
    sbus_state.channels[11] = buffer[16] >> 1 | ((uint16_t)buffer[17] << 7  & 0x07FF);
    sbus_state.channels[12] = buffer[17] >> 4 | ((uint16_t)buffer[18] << 4  & 0x07FF);
    sbus_state.channels[13] = buffer[18] >> 7 |  (uint16_t)buffer[19] << 1
                                              | ((uint16_t)buffer[20] << 9  & 0x07FF);
    sbus_state.channels[14] = buffer[20] >> 2 | ((uint16_t)buffer[21] << 6  & 0x07FF);
    sbus_state.channels[15] = buffer[21] >> 5 | ((uint16_t)buffer[22] << 3  & 0x07FF);

    sbus_state.channel17 = buffer[23] & SBUS_CH17_MASK ? 1 : 0;
    sbus_state.channel18 = buffer[23] & SBUS_CH18_MASK ? 1 : 0;
    sbus_state.lost_frame = buffer[23] & SBUS_LOST_FRAME_MASK ? 1 : 0;
    sbus_state.failsafe = buffer[23] & SBUS_FAILSAFE_MASK ? 1 : 0;

    uint16_t position;
    position = (uint16_t)(0.638 * sbus_state.channels[0] + 857.0);
    servoSetValue(&servos[0], position);
    position = (uint16_t)(0.638 * sbus_state.channels[1] + 857.0);
    servoSetValue(&servos[1], position);
    position = (uint16_t)(0.638 * sbus_state.channels[2] + 857.0);
    servoSetValue(&servos[2], position);
    position = (uint16_t)(0.638 * sbus_state.channels[3] + 857.0);
    servoSetValue(&servos[3], position);
  }
}

/*
 * This callback is invoked when a receive buffer has been completely written.
 */
static void rxend(UARTDriver *uartp) {

  (void)uartp;
}

/*
 * This callback is invoked when configured timeout reached.
 */
static void rxtimeout(UARTDriver *uartp) {

  (void)uartp;
}

static UARTConfig uart_cfg = {
  NULL,
  NULL,
  rxend,
  rxchar,
  rxerr,
  rxtimeout,
  100000,
  USART_CR1_PCE, /* PS bit cleared means even parity.*/
  USART_CR2_STOP_1,
  0
};

/*
 * This callback is invoked when a transmission buffer has been completely
 * read by the driver.
 */
static void txend1(UARTDriver *uartp) {

  (void)uartp;
}

/*
 * This callback is invoked when a transmission has physically completed.
 */
static void txend2(UARTDriver *uartp) {

  (void)uartp;
}

static UARTConfig uart6_cfg = {
  txend1,
  txend2,
  NULL,
  NULL,
  NULL,
  NULL,
  230400,
  0,
  0,
  0
};

static void sbus_init(void) {
  uartStart(&UARTD3, &uart_cfg);
  palSetPadMode(GPIOD, 9, PAL_MODE_ALTERNATE(7));
  uartStart(&UARTD6, &uart6_cfg);
  palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(8));
}

static void sbus_read(void) {
  return;
}

THD_WORKING_AREA(waSbus, 128);
THD_FUNCTION(thSbus, arg) {
  (void)arg;
  chRegSetThreadName("thSbus");

  sbus_init();

  while (true) {
    sbus_read();
    chThdSleepMilliseconds(321);
  }
}

void shellcmd_sbus(BaseSequentialStream *chp, int argc, char *argv[]) {

  if (argc == 0) {
    goto ERROR;
  }

  if (argc == 1) {
    if (strcmp(argv[0], "get") == 0) {
      chprintf(chp, "got %d\r\n", 2);
      return;
    } else if ((argc == 2) && (strcmp(argv[0], "set") == 0)) {
      chprintf(chp, "set\r\n");
      return;
    }
  }

ERROR:
  chprintf(chp, "Usage: sbus get\r\n");
  chprintf(chp, "       sbus set x\r\n");
  chprintf(chp, "where x is something\r\n");
  chprintf(chp, "and that's it\r\n");
  return;
}
