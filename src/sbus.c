/*
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

#include "sbus.h"
#include "chprintf.h"

int sbus = 0;

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

  // TODO: Now do the parsing
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
  sbus++;

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
