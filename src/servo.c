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

#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "servo.h"

static binary_semaphore_t servo_ready_bsem;
static mutex_t servo_data_mtx;
servo_data_t servo_data;

ServoPWM servos[] = {
  {
    &PWMD5,                /* Timer 5.*/
    0,                     /* Channel 0.*/
    GPIOA,                 /* Port A.*/
    0,                     /* Pin PA0.*/
    PAL_MODE_ALTERNATE(2), /* AF 2.*/
    900,                   /* Min value.*/
    2100,                  /* Max value.*/
    1500                   /* Current value.*/
  },
  {
    &PWMD5,
    1,
    GPIOA,
    1, /* Yes, we are reusing GPIOA_LED as a PWM OUTPUT. No good GPIO is left.*/
    PAL_MODE_ALTERNATE(2),
    900,
    2100,
    1500
  },
  {
    &PWMD5,
    2,
    GPIOA,
    2,
    PAL_MODE_ALTERNATE(2),
    900,
    2100,
    1500
  },
  {
    &PWMD5,
    3,
    GPIOA,
    3,
    PAL_MODE_ALTERNATE(2),
    900,
    2100,
    1500
  },
  {
    &PWMD3,
    0,
    GPIOA,
    6,
    PAL_MODE_ALTERNATE(2),
    900,
    2100,
    1500
  },
  {
    &PWMD3,
    1,
    GPIOA,
    7,
    PAL_MODE_ALTERNATE(2),
    900,
    2100,
    1500
  },
  {
    &PWMD3,
    2,
    GPIOB,
    0,
    PAL_MODE_ALTERNATE(2),
    900,
    2100,
    1500
  },
  {
    &PWMD3,
    3,
    GPIOB,
    1,
    PAL_MODE_ALTERNATE(2),
    900,
    2100,
    1500
  }
};

static PWMConfig pwmcfg = {
  1000000, /* Base frequency is 1 MHz.*/
  20000,   /* 20 milliseconds PWM period.*/
  NULL,    /* No callback.*/
  {
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL} /* Up to PWM_CHANNELS.*/
  },       /* Channels configurations.*/
  0,       /* CR2.*/
  0,       /* BDTR.*/
  0        /* DIER.*/
};

void servoInit(ServoPWM *servo)
{
  palSetPadMode(servo->port, servo->pin, servo->mode);

  PWMChannelConfig chcfg = {
    PWM_OUTPUT_ACTIVE_HIGH, /* Channel active logic level.*/
    NULL                    /* Channel callback pointer.*/
  };
  /* Enable channel.*/
  pwmcfg.channels[servo->pwm_channel] = chcfg;
  pwmStart(servo->pwm_driver, &pwmcfg);
  if (servo->position > 0) {
    servoPosition(servo, servo->position);
  }
}

void servoSetMaxPosition(ServoPWM *servo, uint16_t position)
{
  servo->max_position = position;
}

void servoSetMinPosition(ServoPWM *servo, uint16_t position)
{
  servo->min_position = position;
}

void servoMax(ServoPWM *servo)
{
  servoPosition(servo, servo->max_position);
}

void servoMin(ServoPWM *servo)
{
  servoPosition(servo, servo->min_position);
}

void servoMiddle(ServoPWM *servo)
{
  servoPosition(servo, (servo->max_position + servo->min_position) / 2);
}

void servoPosition(ServoPWM *servo, uint16_t position)
{
  if (position > servo->max_position) {
    servo->position = servo->max_position;
  } else if (position < servo->min_position) {
    servo->position = servo->min_position;
  } else {
    servo->position = position;
  }

  pwmEnableChannel(servo->pwm_driver, servo->pwm_channel, (pwmcnt_t)servo->position);
}

void servo_sync_init(void)
{
  chBSemWait(&servo_ready_bsem);
}

void servo_copy_data(servo_data_t *source, servo_data_t *target)
{
  chMtxLock(&servo_data_mtx);
  memcpy(target, source, sizeof(servo_data_t));
  chMtxUnlock(&servo_data_mtx);
}

THD_WORKING_AREA(waServo, SERVO_THREAD_STACK_SIZE);
THD_FUNCTION(thServo, arg)
{
  (void)arg;

  servo_data_t sd;

  chRegSetThreadName("servo");
  chBSemObjectInit(&servo_ready_bsem, true);
  chMtxObjectInit(&servo_data_mtx);

  servoInit(&servos[0]);
  servoInit(&servos[1]);
  servoInit(&servos[2]);
  servoInit(&servos[3]);
  servoInit(&servos[4]);
  servoInit(&servos[5]);
  servoInit(&servos[6]);
  servoInit(&servos[7]);

  chBSemSignal(&servo_ready_bsem);

  while (true) {
    /* FIXME: synchronize with controller in a slightly better way.
       This thread could easily wait for an event or be locked on a binary
       semaphore or something similar.*/
    chThdSleepMilliseconds(10);
    servo_copy_data(&servo_data, &sd);
    servoPosition(&servos[0], sd.servos[0].position);
    servoPosition(&servos[1], sd.servos[1].position);
    servoPosition(&servos[2], sd.servos[2].position);
    servoPosition(&servos[3], sd.servos[3].position);
    servoPosition(&servos[4], sd.servos[4].position);
    servoPosition(&servos[5], sd.servos[5].position);
    servoPosition(&servos[6], sd.servos[6].position);
    servoPosition(&servos[7], sd.servos[7].position);
  }
}

void shellcmd_servo(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chprintf(chp, "%4d %4d %4d %4d %4d %4d %4d %4d\r\n",
             servos[0].position, servos[1].position,
             servos[2].position, servos[3].position,
             servos[4].position, servos[5].position,
             servos[6].position, servos[7].position);
    chThdSleepMilliseconds(50);
  }
}
