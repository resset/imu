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

#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

typedef struct ServoPWM_t {
  PWMDriver *pwm_driver;
  pwmchannel_t pwm_channel;
  ioportid_t port;
  ioportmask_t pin;
  iomode_t mode;
  uint16_t min;
  uint16_t max;
} ServoPWM;

void servoInit(ServoPWM *servo);
void servoSetMin(ServoPWM *servo, uint16_t value);
void servoSetMax(ServoPWM *servo, uint16_t value);
void servoSetValue(ServoPWM *servo, uint16_t value);

extern ServoPWM servos[];

extern THD_WORKING_AREA(waServo, 128);
THD_FUNCTION(thServo, arg);

void shellcmd_servo(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _SERVO_H_ */
