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

#ifndef _BMP280_H_
#define _BMP280_H_

/*
 * Registers.
 */

#define BMP280_DIG_T1_LSB 0x88
#define BMP280_DIG_T1_MSB 0x89
#define BMP280_DIG_T2_LSB 0x8A
#define BMP280_DIG_T2_MSB 0x8B
#define BMP280_DIG_T3_LSB 0x8C
#define BMP280_DIG_T3_MSB 0x8D
#define BMP280_DIG_P1_LSB 0x8E
#define BMP280_DIG_P1_MSB 0x8F
#define BMP280_DIG_P2_LSB 0x90
#define BMP280_DIG_P2_MSB 0x91
#define BMP280_DIG_P3_LSB 0x92
#define BMP280_DIG_P3_MSB 0x93
#define BMP280_DIG_P4_LSB 0x94
#define BMP280_DIG_P4_MSB 0x95
#define BMP280_DIG_P5_LSB 0x96
#define BMP280_DIG_P5_MSB 0x97
#define BMP280_DIG_P6_LSB 0x98
#define BMP280_DIG_P6_MSB 0x99
#define BMP280_DIG_P7_LSB 0x9A
#define BMP280_DIG_P7_MSB 0x9B
#define BMP280_DIG_P8_LSB 0x9C
#define BMP280_DIG_P8_MSB 0x9D
#define BMP280_DIG_P9_LSB 0x9E
#define BMP280_DIG_P9_MSB 0x9F
#define BMP280_ID         0xD0
#define BMP280_RESET      0xE0
#define BMP280_STATUS     0xF3
#define BMP280_CTRL_MEAS  0xF4
#define BMP280_CONFIG     0xF5
#define BMP280_PRESS_MSB  0xF7
#define BMP280_PRESS_LSB  0xF8
#define BMP280_PRESS_XLSB 0xF9
#define BMP280_TEMP_MSB   0xFA
#define BMP280_TEMP_LSB   0xFB
#define BMP280_TEMP_XLSB  0xFC

/*
 * Register values.
 */

#define BMP280_VAL_ID     0x58
#define BMP280_VAL_RESET  0xB6

/*
 * Other constants.
 */

/* If pin SDO is tied to GND then LSB is 0.*/
#define BMP280_ADDR_LOW  0x76
/* If pin SDO is tied to VCC then LSB is 1.*/
#define BMP280_ADDR_HIGH 0x77

#define BMP280_ADC_T_MIN      ((int32_t)0x00000)
#define BMP280_ADC_T_MAX      ((int32_t)0xFFFF0)
#define BMP280_ADC_P_MIN      ((int32_t)0x00000)
#define BMP280_ADC_P_MAX      ((int32_t)0xFFFF0)
#define BMP280_MIN_TEMP_INT   ((int32_t)-4000)
#define BMP280_MAX_TEMP_INT   ((int32_t)8500)
#define BMP280_MIN_PRES_64INT ((uint32_t)(30000 * 256))
#define BMP280_MAX_PRES_64INT ((uint32_t)(110000 * 256))

#endif /* _BMP280_H_ */
