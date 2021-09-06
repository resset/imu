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
