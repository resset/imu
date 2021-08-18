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

#include "hal.h"
#include "system.h"

void system_init(void)
{
  SCB->CCR |= 0x10;

  /* Debug UART.*/
  sdStart(&SD7, NULL);
  palSetPadMode(GPIOE, 8, PAL_MODE_ALTERNATE(7)); /* TX */
  palSetPadMode(GPIOE, 7, PAL_MODE_ALTERNATE(7)); /* RX */

  return;
}

void get_registers_from_stack(uint32_t *registers)
{
  volatile uint32_t r0;
  volatile uint32_t r1;
  volatile uint32_t r2;
  volatile uint32_t r3;
  volatile uint32_t r12;
  volatile uint32_t lr;
  volatile uint32_t pc;
  volatile uint32_t psr;
  volatile uint32_t CFSR;
  volatile uint32_t HFSR;
  volatile uint32_t MMAR;
  volatile uint32_t BFAR;

  r0 = registers[0];
  r1 = registers[1];
  r2 = registers[2];
  r3 = registers[3];

  r12 = registers[4];
  lr = registers[5];
  pc = registers[6];
  psr = registers[7];

  /* Configurable Fault Status Register
     Consists of MMFSR, BFSR and UFSR.*/
  CFSR = (*((volatile uint32_t*)(0xE000ED28)));
  /* Hard Fault Status Register */
  HFSR = (*((volatile uint32_t*)(0xE000ED2C)));
  /* Read the Fault Address Registers. These may not contain valid values.
     Check MMARVALID in MMFSR and BFARVALID in BFSR to see if they are valid
     values.*/
  /* MemManage Fault Address Register */
  MMAR = (*((volatile uint32_t*)(0xE000ED34)));
  /* Bus Fault Address Register */
  BFAR = (*((volatile uint32_t*)(0xE000ED38)));
  /* No AFSR.*/

  (void)r0;
  (void)r1;
  (void)r2;
  (void)r3;
  (void)r12;
  (void)lr;
  (void)pc;
  (void)psr;
  (void)CFSR;
  (void)HFSR;
  (void)MMAR;
  (void)BFAR;

  while (true);
}

void HardFault_Handler(void)
{
  __asm volatile
  (
    " tst lr, #4                                    \n"
    " ite eq                                        \n"
    " mrseq r0, msp                                 \n"
    " mrsne r0, psp                                 \n"
    " ldr r1, [r0, #24]                             \n"
    " ldr r2, get_reg_const                         \n"
    " bx r2                                         \n"
    " get_reg_const: .word get_registers_from_stack \n"
    " bkpt #0                                       \n"
  );
  while (true);
}
