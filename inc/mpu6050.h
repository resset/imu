#ifndef _MPU6050_H_
#define _MPU6050_H_

/* If pin AD0 is tied to GND then LSB is 0.*/
#define MPU6050_ADDR_LOW  0x68
/* If pin AD0 is tied to VCC then LSB is 1.*/
#define MPU6050_ADDR_HIGH 0x69

/*
 * Registers.
 */

#define MPU6050_SIGNAL_PATH_RESET 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_PWR_MGMT_2 0x6C
#define MPU6050_WHO_AM_I 0x75

#endif /* _MPU6050_H_ */
