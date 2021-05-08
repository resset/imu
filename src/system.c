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

  r0 = registers[0];
  r1 = registers[1];
  r2 = registers[2];
  r3 = registers[3];

  r12 = registers[4];
  lr = registers[5];
  pc = registers[6];
  psr = registers[7];

  (void)r0;
  (void)r1;
  (void)r2;
  (void)r3;
  (void)r12;
  (void)lr;
  (void)pc;
  (void)psr;

  while (true);
}

void HardFaultVector(void)
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
  );
  __asm volatile ("BKPT #01");
  while (true);
}
