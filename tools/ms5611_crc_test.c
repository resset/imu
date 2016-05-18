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

/*
 * Compile with:
 * gcc -std=c99 -Wpedantic -Wall -Wshadow -Wextra -o ms5611_crc_test ms5611_crc_test.c
 */

#include <inttypes.h>
#include <stdio.h>

uint8_t ms5611_crc4(uint16_t n_prom[]) {
  uint16_t n_rem;
  uint16_t crc_read;

  n_rem = 0x0;
  crc_read = n_prom[7];
  n_prom[7] = (0xFF00 & (n_prom[7]));

  for (uint8_t cnt = 0; 16 > cnt; ++cnt) {
    if (cnt % 2 == 1) {
        n_rem ^= (uint16_t)((n_prom[cnt >> 1]) & 0x00FF);
    } else  {
        n_rem ^= (uint16_t)(n_prom[cnt >> 1] >> 8);
    }
    for (uint8_t n_bit = 8; 0 < n_bit; --n_bit) {
      if (n_rem & (0x8000)) {
        n_rem = (n_rem << 1) ^ 0x3000;
      } else {
        n_rem = (n_rem << 1);
      }
    }
  }
  n_rem = (0x000F & (n_rem >> 12));
  n_prom[7] = crc_read;

  return n_rem ^ 0x0;
}

int main(void) {
  /* Test data taken from AN520 application note with CRC added to c[7].*/
  uint16_t c[8] = {0x3132, 0x3334, 0x3536, 0x3738, 0x3940, 0x4142, 0x4344, 0x450B};

  /* PROM contents obtained from my MS5611.*/
  /*uint16_t c[8] = {0x0, 0xC471, 0xC783, 0x7878, 0x6F83, 0x7A4F, 0x6CF4, 0xE747};*/

  uint8_t crc, res;

  crc = (uint8_t)(c[7] & 0xF);

  res = ms5611_crc4(c);

  printf("CRC read from PROM: 0x%X\n", crc);
  printf("Calculated CRC:     0x%X\n", res);

  if (crc == res) {
      printf("\nOK, values match\n");
      return 0;
  } else {
      printf("\nERROR, values does not match\n");
      return 1;
  }
}
