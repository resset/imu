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

#include "bar_main.h"

uint16_t c[8]; /* Coefficient table.*/
uint32_t d1;   /* Digital pressure value.*/
uint32_t d2;   /* Digital temperature value.*/
int32_t dt;    /* Difference between actual and reference temperature.*/
int32_t temp;  /* Actual temperature.*/
int64_t off;   /* Offset at actual temperature.*/
int64_t sens;  /* Sensitivity at actual temperature.*/
int64_t p;     /* Temperature compensated pressure.*/

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

static uint8_t bar_init(uint16_t c[]) {
  uint8_t txbuf[1];
  uint8_t rxbuf[2] = {0, 0};
  uint8_t crc_calculated;

  i2c_sensors_init();

  i2cAcquireBus(&I2CD1);

  txbuf[0] = MS5611_CMD_RESET;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 0, MS2ST(0x3000));

  i2cReleaseBus(&I2CD1);

  /* Datasheet of MS5611 says 2.8 ms.*/
  chThdSleepMilliseconds(3);

  i2cAcquireBus(&I2CD1);

  txbuf[0] = MS5611_CMD_READ_RESERVED;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c[0] = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_C1;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c[1] = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_C2;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c[2] = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_C3;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c[3] = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_C4;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c[4] = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_C5;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c[5] = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_C6;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c[6] = (rxbuf[0] << 8) | rxbuf[1];

  txbuf[0] = MS5611_CMD_READ_CRC;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 2, MS2ST(0x3000));
  c[7] = (rxbuf[0] << 8) | rxbuf[1];

  i2cReleaseBus(&I2CD1);

  crc_calculated = ms5611_crc4(c);
  if ((uint8_t)(c[7] & 0xF) != crc_calculated) {
    return 1;
  }

  return 0;
}

static uint32_t get_adc_data(uint8_t command) {
  uint8_t txbuf[1];
  uint8_t rxbuf[3] = {0, 0, 0};

  i2cAcquireBus(&I2CD1);
  txbuf[0] = command;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 0, MS2ST(0x3000));
  i2cReleaseBus(&I2CD1);

  switch (command & 0xF) {
    case MS5611_CMD_CONVERT_256:
      chThdSleepMicroseconds(900);
      break;
    case MS5611_CMD_CONVERT_512:
      chThdSleepMilliseconds(3);
      break;
    case MS5611_CMD_CONVERT_1024:
      chThdSleepMilliseconds(4);
      break;
    case MS5611_CMD_CONVERT_2048:
      chThdSleepMilliseconds(6);
      break;
    case MS5611_CMD_CONVERT_4096:
      chThdSleepMilliseconds(10);
      break;
  }

  i2cAcquireBus(&I2CD1);
  txbuf[0] = MS5611_CMD_READ_ADC;
  i2cMasterTransmitTimeout(&I2CD1, MS5611_I2C_ADDR, txbuf, 1, rxbuf, 3, MS2ST(0x3000));
  i2cReleaseBus(&I2CD1);

  return (rxbuf[0] << 16) | (rxbuf[1] << 8) | rxbuf[2];
}

static void bar_read(void) {

  d1 = get_adc_data(MS5611_CMD_CONVERT_D1 | MS5611_CMD_CONVERT_1024);
  d2 = get_adc_data(MS5611_CMD_CONVERT_D2 | MS5611_CMD_CONVERT_1024);

  dt = d2 - (c[5] << 8);
  temp = 2000 + (((int64_t)dt * (int64_t)c[6]) >> 23);

//  off = (int64_t)c2 * 65536 + ((int64_t)c4 * (int64_t)dt) / 128;
//  sens = (int64_t)c1 * 32768 + ((int64_t)c3 * (int64_t)dt) / 256;

////  // czyjeś
////  off = (int64_t)c2 * 65536 + (int64_t)c4 * dt / 128;
////  sens = (int64_t)c1 * 32768 + (int64_t)c3 * dt / 256;

  // Tu jest błąd! w 32 bitach to jest liczba ujemna. A powinno być liczone w 64...
  // Dostaję -947716096 a powinienem 3347251200. sprawdź sobie reprezentację bitową tych liczb...
  uint32_t rdhi = 0, rdlo = 0;
  asm("umull %0, %1, %2, %3" : "=r" (rdlo), "=r" (rdhi) : "r" (c[2]), "r" (65536));
  off = ((uint64_t)rdhi << 32) | rdlo;
//  off = (uint64_t)c2 * (uint64_t)65536;
  sens = ((int64_t)c[4] * (int64_t)dt) / 128;

//  if (temp < 2000) {
//    int temp2 = 0;
//    int off2 = 0;
//    int sens2 = 0;
//
//    // stuff...
//
//    if (temp < -1500) {
//    }
//
//    temp -= temp2;
//    off -= off2;
//    sens -= sens2;
//  }

//  p = ((((int64_t)d1 * sens) >> 21) - off) >> 15;

  return;
}

THD_WORKING_AREA(waBar, BAR_THREAD_STACK_SIZE);
THD_FUNCTION(thBar, arg) {
  (void)arg;
  chRegSetThreadName("thBar");

  if (0 != bar_init(c)) {
  }

  while (true) {
    bar_read();
    chThdSleepMilliseconds(500);
  }
}
