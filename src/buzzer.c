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

#include "ch.h"
#include "hal.h"

#include "buzzer.h"
#include "servo.h"

static ServoPWM buzzer = {
  &PWMD4,
  2,
  GPIOD,
  14,
  PAL_MODE_ALTERNATE(2),
  1,
  20000,
  10000
};

static PWMConfig pwmcfg = {
  1000000, /* Base frequency is 1 MHz.*/
  20000,     /* 416 microseconds PWM period.*/
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

THD_WORKING_AREA(waBuzzer, BUZZER_THREAD_STACK_SIZE);
THD_FUNCTION(thBuzzer, arg)
{
  (void)arg;

  chRegSetThreadName("buzzer");

  palSetPadMode(buzzer.port, buzzer.pin, buzzer.mode);
  PWMChannelConfig chcfg = {
    PWM_OUTPUT_ACTIVE_HIGH, /* Channel active logic level.*/
    NULL                    /* Channel callback pointer.*/
  };
  /* Enable channel.*/
  pwmcfg.channels[buzzer.pwm_channel] = chcfg;
  pwmStart(buzzer.pwm_driver, &pwmcfg);
  if (buzzer.position > 0) {
    pwmEnableChannel(buzzer.pwm_driver, buzzer.pwm_channel, (pwmcnt_t)buzzer.position);
  }

  while (true) {
    chThdSleepMilliseconds(100);
    pwmDisableChannel(buzzer.pwm_driver, buzzer.pwm_channel);
  }
}
