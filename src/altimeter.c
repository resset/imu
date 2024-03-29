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

#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "pg.h"
#include "altimeter.h"

typedef enum {
  ALTIMETER_STATE_INIT,
  ALTIMETER_STATE_NOP,
  ALTIMETER_STATE_ZERO,
  ALTIMETER_STATE_READY,
  ALTIMETER_FATAL_ERROR
} altimeter_state_t;

#define ALTIMETER_STATE_NAMES "INIT", "NOP", "ZERO", "READY", "ERROR"

static altimeter_state_t altimeter_state;
static mutex_t altimeter_state_mtx;

typedef struct {
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;
  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;
  int32_t t_adc;
  int32_t p_adc;
  int32_t t_fine;
  int32_t temperature_raw;
  uint32_t pressure_raw;
} bmp280_data_t;

static bmp280_data_t bmp280_data;

static binary_semaphore_t altimeter_ready_bsem;
static mutex_t altimeter_data_mtx;
altimeter_data_t altimeter_data;

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
  i2cMasterTransmitTimeout(&I2CD1, BMP280_ADDR, txbuf, txbuf_len, rxbuf, rxbuf_len, TIME_INFINITE);
}

static void i2c_send(uint8_t *txbuf, size_t txbuf_len)
{
  cacheBufferFlush(txbuf, CACHE_SIZE_ALIGN(uint8_t, txbuf_len));
  i2cMasterTransmitTimeout(&I2CD1, BMP280_ADDR, txbuf, txbuf_len, NULL, 0, TIME_INFINITE);
}

static pg_result_t altimeter_init(bmp280_data_t *bd)
{
  CC_ALIGN_DATA(32) uint8_t txbuf[CACHE_SIZE_ALIGN(uint8_t, 2)];
  CC_ALIGN_DATA(32) uint8_t rxbuf[CACHE_SIZE_ALIGN(uint8_t, 24)];

  /*
   * I2C initialization.
   */
  palSetPadMode(GPIOB, 8, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN); /* SCL */
  palSetPadMode(GPIOB, 7, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN); /* SDA */

  i2cStart(&I2CD1, &i2ccfg);

  i2cAcquireBus(&I2CD1);

  /* Force power-on reset.*/
  txbuf[0] = BMP280_RESET;
  txbuf[1] = BMP280_VAL_RESET;
  i2c_send(txbuf, 2);
  chThdSleepMilliseconds(100);

  /* Test for BMP280.*/
  txbuf[0] = BMP280_ID;
  i2c_transmit(txbuf, 1, rxbuf, 1);

  if (rxbuf[0] != BMP280_VAL_ID) {
    i2cReleaseBus(&I2CD1);
    return PG_ERROR;
  }

  /* config register
     t_sb = 000 (0.5 ms)
     filter = 111 (x16) (0 = OFF, 1 = x2, ... >=4 = x16)
     Bit 1 cannot be written, hence this read before the write.

     NOTE: we have to use aligned memory in i2c_transmit/i2c_send,
     this is why the operations are performed on rxbuf.
  */
  txbuf[0] = BMP280_CONFIG;
  i2c_transmit(txbuf, 1, rxbuf, 1);
  rxbuf[0] = rxbuf[0] & 0x03;
  txbuf[0] = BMP280_CONFIG;
  txbuf[1] = 0x1C | rxbuf[0];
  i2c_send(txbuf, 2);

  /* ctrl_meas register
     osrs_t = 010 (x2, 17-bit, 0.0025 deg. C)
     osrs_p = 101 (x16, 20-bit, 0.16 Pa)
     mode = 11 (normal)
     With settings like this and IIR filter coefficient equaling 16,
     output data rate is 26.3 Hz.
  */
  txbuf[0] = BMP280_CTRL_MEAS;
  txbuf[1] = 0x57;
  i2c_send(txbuf, 2);

  /* Calibration data readout.*/
  txbuf[0] = BMP280_DIG_T1_LSB;
  i2c_transmit(txbuf, 1, rxbuf, 24);
  bd->dig_T1 = ((uint16_t)rxbuf[1]) << 8 | rxbuf[0];
  bd->dig_T2 = ((uint16_t)rxbuf[3]) << 8 | rxbuf[2];
  bd->dig_T3 = ((uint16_t)rxbuf[5]) << 8 | rxbuf[4];
  bd->dig_P1 = ((uint16_t)rxbuf[7]) << 8 | rxbuf[6];
  bd->dig_P2 = ((uint16_t)rxbuf[9]) << 8 | rxbuf[8];
  bd->dig_P3 = ((uint16_t)rxbuf[11]) << 8 | rxbuf[10];
  bd->dig_P4 = ((uint16_t)rxbuf[13]) << 8 | rxbuf[12];
  bd->dig_P5 = ((uint16_t)rxbuf[15]) << 8 | rxbuf[14];
  bd->dig_P6 = ((uint16_t)rxbuf[17]) << 8 | rxbuf[16];
  bd->dig_P7 = ((uint16_t)rxbuf[19]) << 8 | rxbuf[18];
  bd->dig_P8 = ((uint16_t)rxbuf[21]) << 8 | rxbuf[20];
  bd->dig_P9 = ((uint16_t)rxbuf[23]) << 8 | rxbuf[22];

  i2cReleaseBus(&I2CD1);

  return PG_OK;
}

static pg_result_t bmp280_check_boundaries(bmp280_data_t *bd)
{
  /* We check here the sanity of the ADC readings.*/
  if (bd->p_adc < BMP280_ADC_P_MIN || bd->p_adc > BMP280_ADC_P_MAX ||
      bd->t_adc < BMP280_ADC_T_MIN || bd->t_adc > BMP280_ADC_T_MAX) {
    return PG_ERROR;
  } else {
    return PG_OK;
  }
}

static pg_result_t bmp280_compensate_temperature(bmp280_data_t *bd)
{
  pg_result_t ret;
  int32_t var1, var2;

  var1 = ((((bd->t_adc >> 3) - ((int32_t)bd->dig_T1 << 1))) * ((int32_t)bd->dig_T2)) >> 11;
  var2 = (((((bd->t_adc >> 4) - ((int32_t)bd->dig_T1))
            * ((bd->t_adc >> 4) - ((int32_t)bd->dig_T1))) >> 12)
          * ((int32_t)bd->dig_T3)) >> 14;
  bd->t_fine = var1 + var2;
  bd->temperature_raw = (bd->t_fine * 5 + 128) >> 8;

  if (bd->temperature_raw < BMP280_MIN_TEMP_INT) {
    bd->temperature_raw = BMP280_MIN_TEMP_INT;
    ret = PG_ERROR;
  } else if (bd->temperature_raw > BMP280_MAX_TEMP_INT) {
    bd->temperature_raw = BMP280_MAX_TEMP_INT;
    ret = PG_ERROR;
  } else {
    ret = PG_OK;
  }

  return ret;
}

static pg_result_t bmp280_compensate_pressure(bmp280_data_t *bd)
{
  pg_result_t ret;
  int64_t var1, var2, p;

  var1 = ((int64_t)bd->t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)bd->dig_P6;
  var2 = var2 + ((var1 * (int64_t)bd->dig_P5) << 17);
  var2 = var2 + (((int64_t)bd->dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)bd->dig_P3) >> 8) + ((var1 * (int64_t)bd->dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)bd->dig_P1) >> 33;
  if (var1 == 0) {
    bd->pressure_raw = 0;
    return PG_ERROR;
  }
  p = 1048576 - bd->p_adc;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)bd->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)bd->dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)bd->dig_P7) << 4);
  bd->pressure_raw = (uint32_t)p;

  if (bd->pressure_raw < BMP280_MIN_PRES_64INT) {
    bd->pressure_raw = BMP280_MIN_PRES_64INT;
    ret = PG_ERROR;
  } else if (bd->pressure_raw > BMP280_MAX_PRES_64INT) {
    bd->pressure_raw = BMP280_MAX_PRES_64INT;
    ret = PG_ERROR;
  } else {
    ret = PG_OK;
  }

  return ret;
}

static pg_result_t altimeter_read(bmp280_data_t *bd, altimeter_data_t *ad)
{
  CC_ALIGN_DATA(32) uint8_t txbuf[CACHE_SIZE_ALIGN(uint8_t, 2)];
  CC_ALIGN_DATA(32) uint8_t rxbuf[CACHE_SIZE_ALIGN(uint8_t, 24)];

  ad->data_valid = false;

  txbuf[0] = BMP280_PRESS_MSB;

  i2cAcquireBus(&I2CD1);
  i2c_transmit(txbuf, 1, rxbuf, 6);
  i2cReleaseBus(&I2CD1);

  bd->t_adc = (int32_t)rxbuf[3] << 12 | (int32_t)rxbuf[4] << 4 | (int32_t)rxbuf[5] >> 4;
  bd->p_adc = (int32_t)rxbuf[0] << 12 | (int32_t)rxbuf[1] << 4 | (int32_t)rxbuf[2] >> 4;

  if (bmp280_check_boundaries(bd) == PG_OK) {
    if (bmp280_compensate_temperature(bd) == PG_OK) {
      if (bmp280_compensate_pressure(bd) == PG_OK) {
        ad->temperature = (float)bd->temperature_raw / 100.0f;
        ad->pressure = (float)bd->pressure_raw / 256.0f;
        return PG_OK;
      }
    }
  }

  return PG_ERROR;
}

static pg_result_t altimeter_zero(bmp280_data_t *bd, altimeter_data_t *ad)
{
  uint16_t i = 0, guard = 0;
  float p_reference = 0.0f;

  while (i < PG_CFG_ALT_ZERO_SAMPLES && guard != PG_CFG_ALT_ZERO_MAX_TRIES) {
    if (altimeter_read(bd, ad) == PG_OK) {
      p_reference += ad->pressure;
      i++;
    }
    guard++;
  }

  if (guard != PG_CFG_ALT_ZERO_MAX_TRIES) {
    ad->pressure_reference = p_reference / (float)PG_CFG_ALT_ZERO_SAMPLES;
    return PG_OK;
  } else {
    ad->pressure_reference = 0.0f;
    return PG_ERROR;
  }
}

static pg_result_t altimeter_altitude(altimeter_data_t *ad)
{
  /* We calculate altitude regardless of the data validity, the valid flag
     is common for all data so it is only true after successful altitude
     calculation.*/
  if (ad->pressure_reference != 0.0f) {
    ad->altitude = (
        (powf(ad->pressure / ad->pressure_reference, 0.1902665f) - 1)
        *
        (ad->temperature + 273.15f)
      ) / (-0.0065f);

    ad->data_valid = true;
    return PG_OK;
  } else {
    return PG_ERROR;
  }
}

pg_result_t altimeter_state_zero(void)
{
  chMtxLock(&altimeter_state_mtx);
  if (altimeter_state == ALTIMETER_STATE_READY) {
    altimeter_state = ALTIMETER_STATE_ZERO;
    chMtxUnlock(&altimeter_state_mtx);
    return PG_OK;
  } else {
    chMtxUnlock(&altimeter_state_mtx);
    return PG_ERROR;
  }
}

altimeter_state_t altimeter_state_get(void)
{
  altimeter_state_t state;

  chMtxLock(&altimeter_state_mtx);
  state = altimeter_state;
  chMtxUnlock(&altimeter_state_mtx);

  return state;
}

void altimeter_state_set(altimeter_state_t state)
{
  chMtxLock(&altimeter_state_mtx);
  altimeter_state = state;
  chMtxUnlock(&altimeter_state_mtx);
}

void altimeter_sync_init(void)
{
  chBSemWait(&altimeter_ready_bsem);
}

void altimeter_copy_data(altimeter_data_t *source, altimeter_data_t *target)
{
  chMtxLock(&altimeter_data_mtx);
  memcpy(target, source, sizeof(altimeter_data_t));
  chMtxUnlock(&altimeter_data_mtx);
}

THD_WORKING_AREA(waAltimeter, ALTIMETER_THREAD_STACK_SIZE);
THD_FUNCTION(thAltimeter, arg)
{
  (void)arg;
  altimeter_data_t ad;
  ad.data_valid = false;
  ad.pressure_reference = 0.0f;

  chRegSetThreadName("altimeter");

  chBSemObjectInit(&altimeter_ready_bsem, true);
  chMtxObjectInit(&altimeter_data_mtx);
  chMtxObjectInit(&altimeter_state_mtx);

  while (true) {
    switch (altimeter_state_get()) {
      case ALTIMETER_STATE_INIT:
        if (altimeter_init(&bmp280_data) == PG_OK) {
#ifdef PG_CFG_ALT_ZERO_ON_INIT
        altimeter_state_set(ALTIMETER_STATE_ZERO);
#else
        altimeter_state_set(ALTIMETER_STATE_NOP);
#endif
        } else {
          altimeter_state_set(ALTIMETER_FATAL_ERROR);
        }
        break;
      case ALTIMETER_STATE_NOP:
        break;
      case ALTIMETER_STATE_ZERO:
        if (altimeter_zero(&bmp280_data, &ad) == PG_OK) {
          /* When signalling, we should make sure that all data are present.*/
          altimeter_altitude(&ad);
          altimeter_copy_data(&ad, &altimeter_data);

          altimeter_state_set(ALTIMETER_STATE_READY);
          chBSemSignal(&altimeter_ready_bsem);
        } else {
          /* TODO: we shoud probably try few more times.*/
          altimeter_state_set(ALTIMETER_FATAL_ERROR);
        }
        break;
      case ALTIMETER_STATE_READY:
        if (altimeter_read(&bmp280_data, &ad) != PG_OK) {
          /* We do not want to fail here. We should rather mark the data as
             invalid so that the controller decides on what to do. We should
             then attempt to compute the next sample.*/
        }
        altimeter_altitude(&ad);
        altimeter_copy_data(&ad, &altimeter_data);
        break;
      case ALTIMETER_FATAL_ERROR:
        return;
    }

    /* TODO: the schedule of this thread has to be put into greater context
       of all subprocesses cooperation, of course.*/
    chThdSleepMilliseconds(50);
  }
}

void shellcmd_altimeter(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  altimeter_data_t ad;

  /* Currently we are allowed to check once, because READY state is a dead-end.*/
  if (altimeter_state_get() == ALTIMETER_STATE_READY) {
    while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
      altimeter_copy_data(&altimeter_data, &ad);
      chprintf(chp, "%6d deg. C / 100, %6d, Pa %6d cm\r\n",
              (int32_t)(ad.temperature * 100.0f),
              (int32_t)ad.pressure,
              (int32_t)(ad.altitude * 100.0f));
      chThdSleepMilliseconds(100);
    }
  }
}
