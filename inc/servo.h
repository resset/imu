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

#ifndef _SERVO_H_
#define _SERVO_H_

#include "ch.h"
#include "hal.h"

#define SERVO_QUANTITY 4

typedef struct ServoPWM_s {
  PWMDriver *pwm_driver;
  pwmchannel_t pwm_channel;
  ioportid_t port;
  ioportmask_t pin;
  iomode_t mode;
  uint16_t min_position;
  uint16_t max_position;
  uint16_t position;
} ServoPWM;

extern ServoPWM servos[];

typedef struct {
  struct {
    uint16_t position;
 } servos[SERVO_QUANTITY];
} servo_data_t;

extern binary_semaphore_t servo_ready_bsem;
extern mutex_t servo_data_mtx;
extern servo_data_t servo_data;

void servo_copy_data(servo_data_t *source, servo_data_t *target);

void servoInit(ServoPWM *servo);
void servoSetMinPosition(ServoPWM *servo, uint16_t position);
void servoSetMaxPosition(ServoPWM *servo, uint16_t position);
void servoMax(ServoPWM *servo);
void servoMin(ServoPWM *servo);
void servoMiddle(ServoPWM *servo);
void servoPosition(ServoPWM *servo, uint16_t position);

extern THD_WORKING_AREA(waServo, 128);
THD_FUNCTION(thServo, arg);

void shellcmd_servo(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _SERVO_H_ */
