#include <stdint.h>
#include <stdio.h>

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
  int32_t t_fine;
  int32_t p_adc;
  int32_t temperature;
  uint32_t pressure;
  int32_t altitude;
} altimeter_data_t;

static int32_t bmp280_compensate_temperature(altimeter_data_t *ad)
{
  int32_t var1, var2;

  var1 = ((((ad->t_adc >> 3) - ((int32_t)ad->dig_T1 << 1))) * ((int32_t)ad->dig_T2)) >> 11;
  var2 = (((((ad->t_adc >> 4) - ((int32_t)ad->dig_T1)) * ((ad->t_adc >> 4) - ((int32_t)ad->dig_T1))) >> 12)
          * ((int32_t)ad->dig_T3)) >> 14;
  ad->t_fine = var1 + var2;
  ad->temperature = (ad->t_fine * 5 + 128) >> 8;
  return ad->temperature;
}

static uint32_t bmp280_compensate_pressure(altimeter_data_t *ad)
{
  int64_t var1, var2, p;

  var1 = ((int64_t)ad->t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)ad->dig_P6;
  var2 = var2 + ((var1 * (int64_t)ad->dig_P5) << 17);
  var2 = var2 + (((int64_t)ad->dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)ad->dig_P3) >> 8) + ((var1 * (int64_t)ad->dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)ad->dig_P1) >> 33;
  if (var1 == 0) {
    ad->pressure = 0;
    return 0;
  }
  p = 1048576 - ad->p_adc;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)ad->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)ad->dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)ad->dig_P7) << 4);
  ad->pressure = (uint32_t)p;

  return ad->pressure;
}

int main(void)
{
  altimeter_data_t altimeter_data;

  // altimeter_data.dig_T1 = 27504;
  // altimeter_data.dig_T2 = 26435;
  // altimeter_data.dig_T3 = -1000;
  // altimeter_data.dig_P1 = 36477;
  // altimeter_data.dig_P2 = -10685;
  // altimeter_data.dig_P3 = 3024;
  // altimeter_data.dig_P4 = 2855;
  // altimeter_data.dig_P5 = 140;
  // altimeter_data.dig_P6 = -7;
  // altimeter_data.dig_P7 = 15500;
  // altimeter_data.dig_P8 = -14600;
  // altimeter_data.dig_P9 = 6000;
  // altimeter_data.t_adc = 415148;
  // altimeter_data.p_adc = 519888;

  altimeter_data.dig_T1 = 27377;
  altimeter_data.dig_T2 = 25596;
  altimeter_data.dig_T3 = 50;
  altimeter_data.dig_P1 = 37748;
  altimeter_data.dig_P2 = -10650;
  altimeter_data.dig_P3 = 3024;
  altimeter_data.dig_P4 = 7972;
  altimeter_data.dig_P5 = -212;
  altimeter_data.dig_P6 = -7;
  altimeter_data.dig_P7 = 15500;
  altimeter_data.dig_P8 = -14600;
  altimeter_data.dig_P9 = 6000;
  altimeter_data.t_adc = 533744;
  altimeter_data.p_adc = 325383;

  bmp280_compensate_temperature(&altimeter_data);
  bmp280_compensate_pressure(&altimeter_data);

  printf("### INT ###\n");
  printf("temperature: %d [deg. C * 100]\n", altimeter_data.temperature);
  printf("pressure:    %d [Pa * 256]\n", altimeter_data.pressure);
  printf("pressure:    %d [Pa]\n", altimeter_data.pressure / 256);
}
