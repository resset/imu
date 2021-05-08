/*
    IMU - Copyright (C) 2014-2016 Mateusz Tomaszkiewicz

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

#include "servo.h"
#include "chprintf.h"

ServoPWM servos[] = {
  {
    &PWMD3,                /* Timer 3.*/
    0,                     /* Channel 0.*/
    GPIOB,                 /* Port B.*/
    4,                     /* Pin PB4.*/
    PAL_MODE_ALTERNATE(2), /* AF 2.*/
    1000,                  /* Min value.*/
    2000                   /* Max value.*/
  },
  {
    &PWMD3,
    1,
    GPIOB,
    5,
    PAL_MODE_ALTERNATE(2),
    1000,
    2000
  },
  {
    &PWMD3,
    2,
    GPIOB,
    0,
    PAL_MODE_ALTERNATE(2),
    1000,
    2000
  },
  {
    &PWMD3,
    3,
    GPIOB,
    1,
    PAL_MODE_ALTERNATE(2),
    1000,
    2000
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
  0        /* DIER.*/
};

void servoInit(ServoPWM *servo)
{
  PWMChannelConfig chcfg = {
    PWM_OUTPUT_ACTIVE_HIGH, /* Channel active logic level.*/
    NULL                    /* Channel callback pointer.*/
  };
  pwmcfg.channels[servo->pwm_channel] = chcfg; // ?

  palSetPadMode(servo->port, servo->pin, servo->mode);

  pwmStart(servo->pwm_driver, &pwmcfg);
}

void servoSetValue(ServoPWM *servo, uint16_t value)
{
  if (value > servo->max) {
    value = servo->max;
  } else if (value < servo->min) {
    value = servo->min;
  }

  pwmEnableChannel(servo->pwm_driver, servo->pwm_channel, (pwmcnt_t)value);
}

void servoSetMax(ServoPWM *servo, uint16_t value)
{
  servo->max = value;
}

void servoSetMin(ServoPWM *servo, uint16_t value)
{
  servo->min = value;
}

static void servo_init(void)
{
  servoInit(&servos[0]);
  servoInit(&servos[1]);
  servoInit(&servos[2]);
  servoInit(&servos[3]);
}

static void servo_read(void)
{
  static uint16_t position = 1500;

  //servoSetValue(&servos[0], position);
  position = (position + 100) % 2000;

  return;
}

THD_WORKING_AREA(waServo, 128);
THD_FUNCTION(thServo, arg) {
  (void)arg;
  chRegSetThreadName("thServo");

  servo_init();

  while (true) {
    servo_read();
    chThdSleepMilliseconds(1000);
  }
}

void shellcmd_servo(BaseSequentialStream *chp, int argc, char *argv[]) {

  if (argc == 0) {
    goto ERROR;
  }

  if (argc == 1) {
    if (strcmp(argv[0], "get") == 0) {
      chprintf(chp, "got %d\r\n", 2);
      return;
    } else if ((argc == 2) && (strcmp(argv[0], "set") == 0)) {
      chprintf(chp, "set\r\n");
      return;
    }
  }

ERROR:
  chprintf(chp, "Usage: servo get\r\n");
  chprintf(chp, "       servo set x\r\n");
  chprintf(chp, "where x is something\r\n");
  chprintf(chp, "and that's it\r\n");
  return;
}
