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

#ifndef _ICM20689_H_
#define _ICM20689_H_

/*
 * Registers.
 */

#define ICM20689_SMPLRT_DIV 0x19
#define ICM20689_CONFIG 0x1A
#define ICM20689_GYRO_CONFIG 0x1B
#define ICM20689_ACCEL_CONFIG 0x1C
#define ICM20689_ACCEL_CONFIG2 0x1D
#define ICM20689_FIFO_EN 0x23
#define ICM20689_INT_ENABLE 0x38
#define ICM20689_ACCEL_XOUT_H 0x3B
#define ICM20689_ACCEL_XOUT_L 0x3C
#define ICM20689_ACCEL_YOUT_H 0x3D
#define ICM20689_ACCEL_YOUT_L 0x3E
#define ICM20689_ACCEL_ZOUT_H 0x3F
#define ICM20689_ACCEL_ZOUT_L 0x40
#define ICM20689_TEMP_OUT_H 0x41
#define ICM20689_TEMP_OUT_L 0x42
#define ICM20689_GYRO_XOUT_H 0x43
#define ICM20689_GYRO_XOUT_L 0x44
#define ICM20689_GYRO_YOUT_H 0x45
#define ICM20689_GYRO_YOUT_L 0x46
#define ICM20689_GYRO_ZOUT_H 0x47
#define ICM20689_GYRO_ZOUT_L 0x48
#define ICM20689_SIGNAL_PATH_RESET 0x68
#define ICM20689_USER_CTRL 0x6A
#define ICM20689_PWR_MGMT_1 0x6B
#define ICM20689_PWR_MGMT_2 0x6C
#define ICM20689_WHO_AM_I 0x75

/*
 * Register values.
 */

#define ICM20689_VAL_WHO_AM_I 0x98

/*
 * Other constants.
 */

#define ICM20689_READ_MASK  0x80
#define ICM20689_WRITE_MASK 0x00

#endif /* _ICM20689_H_ */
