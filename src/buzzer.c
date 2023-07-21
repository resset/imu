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

#define EVT_QUINDAR EVENT_MASK(0)

static thread_t *buzzer_thread = NULL;
static binary_semaphore_t buzzer_ready_bsem;

static ServoPWM buzzer = {
  &PWMD4,
  2,
  GPIOD,
  14,
  PAL_MODE_ALTERNATE(2),
  0,
  0,
  198
};

static PWMConfig pwmcfg = {
  1000000,
  396,
  NULL,
  {
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL}
  },
  0,
  0,
  0
};

void buzzer_play_quindar_tones(void)
{
  chEvtSignal(buzzer_thread, EVT_QUINDAR);
}

static void _buzzer_quindar_tones(void)
{
  /* Original Quindar tones used in Apollo comms were 250 ms long single
     frequency sounds. Intro tone was a pure sine of 2,525 Hz and outro was
     2,475 Hz. Here we have our own version: outro and intro tones, played one
     after the other for 250 ms both with a small gap in between. We output
     square wave but it sounds decent and familiar.

     Base clock of our timer is 1 MHz. Below we set period to 404 which gives
     frequency of around 1000000 / 404 = 2475 Hz. We set width to 202 which is
     half of that time, because we want to produce 50% duty cycle square wave.
     The same calculation is valid for other tone.*/
  /* Outro: 2475 Hz.*/
  pwmcfg.period = 404;
  pwmStart(buzzer.pwm_driver, &pwmcfg);
  pwmEnableChannel(buzzer.pwm_driver, buzzer.pwm_channel, (pwmcnt_t)202);
  chThdSleepMilliseconds(115);
  pwmDisableChannel(buzzer.pwm_driver, buzzer.pwm_channel);
  chThdSleepMilliseconds(20);
  /* Intro: 2525 Hz.*/
  pwmcfg.period = 396;
  pwmStart(buzzer.pwm_driver, &pwmcfg);
  pwmEnableChannel(buzzer.pwm_driver, buzzer.pwm_channel, (pwmcnt_t)198);
  chThdSleepMilliseconds(115);
  pwmDisableChannel(buzzer.pwm_driver, buzzer.pwm_channel);
}

void buzzer_sync_init(void)
{
  chBSemWait(&buzzer_ready_bsem);
}

THD_WORKING_AREA(waBuzzer, BUZZER_THREAD_STACK_SIZE);
THD_FUNCTION(thBuzzer, arg)
{
  (void)arg;

  eventmask_t evt;

  chRegSetThreadName("buzzer");
  buzzer_thread = chThdGetSelfX();
  chBSemObjectInit(&buzzer_ready_bsem, true);

  palSetPadMode(buzzer.port, buzzer.pin, buzzer.mode);
  PWMChannelConfig chcfg = {
    PWM_OUTPUT_ACTIVE_HIGH, /* Channel active logic level.*/
    NULL                    /* Channel callback pointer.*/
  };
  /* Enable channel.*/
  pwmcfg.channels[buzzer.pwm_channel] = chcfg;

  chBSemSignal(&buzzer_ready_bsem);

  while (true) {
    evt = chEvtWaitAny(ALL_EVENTS);

    if (evt & EVT_QUINDAR) {
      /* Actually play Quindar tones.*/
      _buzzer_quindar_tones();
    }
  }
}

void shellcmd_buzz(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;

  buzzer_play_quindar_tones();
}
