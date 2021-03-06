#ifndef _MS5611_H_
#define _MS5611_H_

/* If pin nCSB is tied to VCC then LSB is 0.*/
#define MS5611_I2C_ADDR_HIGH 0x76
/* If pin nCSB is tied to GND then LSB is 1.*/
#define MS5611_I2C_ADDR_LOW  0x77

/*
 * Commands.
 */

#define MS5611_CMD_RESET 0x1E

/* D1 conversion.*/
#define MS5611_CMD_CONVERT_D1      0x40
#define MS5611_CMD_CONVERT_D1_256  0x40
#define MS5611_CMD_CONVERT_D1_512  0x42
#define MS5611_CMD_CONVERT_D1_1024 0x44
#define MS5611_CMD_CONVERT_D1_2048 0x46
#define MS5611_CMD_CONVERT_D1_4096 0x48

/* D2 conversion.*/
#define MS5611_CMD_CONVERT_D2      0x50
#define MS5611_CMD_CONVERT_D2_256  0x50
#define MS5611_CMD_CONVERT_D2_512  0x52
#define MS5611_CMD_CONVERT_D2_1024 0x54
#define MS5611_CMD_CONVERT_D2_2048 0x56
#define MS5611_CMD_CONVERT_D2_4096 0x58

/*
 * Conversion oversampling ratio.
 * Sample combination: MS5611_CMD_CONVERT_D2 | MS5611_CMD_CONVERT_512
 */
#define MS5611_CMD_CONVERT_256     0x0
#define MS5611_CMD_CONVERT_512     0x2
#define MS5611_CMD_CONVERT_1024    0x4
#define MS5611_CMD_CONVERT_2048    0x6
#define MS5611_CMD_CONVERT_4096    0x8

#define MS5611_CMD_READ_ADC 0x0

/* 16 bits reserved for manufacturer.*/
#define MS5611_CMD_READ_RESERVED 0xA0
/* Pressure sensitivity (SENS T1).*/
#define MS5611_CMD_READ_C1       0xA2
/* Pressure offset (OFF T1).*/
#define MS5611_CMD_READ_C2       0xA4
/* Temperature coefficient of pressure sensitivity (TCS).*/
#define MS5611_CMD_READ_C3       0xA6
/* Temperature coefficient of pressure offset (TCO).*/
#define MS5611_CMD_READ_C4       0xA8
/* Reference temperature (TREF).*/
#define MS5611_CMD_READ_C5       0xAA
/* Temperature coefficient of the temperature (TEMPSENS).*/
#define MS5611_CMD_READ_C6       0xAC
/* Cyclic redundancy check sum of the PROM memory (CRC).*/
#define MS5611_CMD_READ_CRC      0xAE

#endif /* _MS5611_H_ */
