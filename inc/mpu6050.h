#ifndef _MPU6050_H_
#define _MPU6050_H_

/* If pin AD0 is tied to GND then LSB is 0.*/
#define MPU6050_ADDR_LOW  0x68
/* If pin AD0 is tied to VCC then LSB is 1.*/
#define MPU6050_ADDR_HIGH 0x69

/*
 * Registers.
 */

#define MPU6050_SMPRT_DIV 0x19
#define MPU6050_CONFIG 0x1A
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_FIFO_EN 0x23
#define MPU6050_INT_ENABLE 0x38
#define MPU6050_SIGNAL_PATH_RESET 0x68
#define MPU6050_USER_CTRL 0x6A
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_PWR_MGMT_2 0x6C
#define MPU6050_WHO_AM_I 0x75
#define MPU6050_WHO_AM_I_IDENTITY 0x68

#endif /* _MPU6050_H_ */
