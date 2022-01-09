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

#include "controller.h"
#include "blackbox.h"
#include "buzzer.h"
#include "display.h"
#include "servo.h"
#include "ground_control.h"
#include "altimeter.h"
#include "gnss.h"
#include "imu.h"

typedef enum {
  CONTROLLER_STATE_INIT,
  CONTROLLER_STATE_WAIT,
  CONTROLLER_STATE_READY,
  CONTROLLER_FATAL_ERROR
} controller_state_t;

static controller_state_t controller_state = CONTROLLER_STATE_INIT;

static void simple_mixer(ground_control_data_t *gcd, servo_data_t *sd)
{
  /* Example channel values change from 224 to 1759 for servos and up to 1793 for RSSI.
     Neutral position is around 992.
     p1 = (224, 900)
     p2 = (1759, 2100)
     Actual 'a' after IEEE-754 conversion is 0.7817590236663818359375, 'b' is 724.885986328.
     */
  for (uint8_t i = 0; i < SERVO_QUANTITY; i++) {
    sd->servos[i].position = (uint16_t)(0.781759 * gcd->channels[i] + 724.885993);
  }
}

static void controller_loop(void)
{
  ground_control_data_t c_ground_control_data;
  altimeter_data_t c_altimeter_data;
  /*gnss_data_t c_gnss_data;*/
  /*imu_data_t c_imu_data;*/
  servo_data_t c_servo_data;
  /*blackbox_data_t c_blackbox_data;*/

  /* Gather sensor data.*/
  ground_control_copy_data(&ground_control_data, &c_ground_control_data);
  altimeter_copy_data(&altimeter_data, &c_altimeter_data);
  /*gnss_copy_data(&gnss_data, &c_gnss_data);*/
  /*imu_copy_data(&imu_data, &c_imu_data);*/

  /* Process data.*/
  simple_mixer(&c_ground_control_data, &c_servo_data);

  /* Output control signals.*/
  servo_copy_data(&c_servo_data, &servo_data);

  /* Output logs.*/
  /*blackbox_copy_data(&c_blackbox_data_t, &blackbox_data_t);*/
}

THD_WORKING_AREA(waController, CONTROLLER_THREAD_STACK_SIZE);
THD_FUNCTION(thController, arg)
{
  (void)arg;

  chRegSetThreadName("controller");

  controller_state = CONTROLLER_STATE_WAIT;

  ground_control_sync_init();
  altimeter_sync_init();
  gnss_sync_init();
  /*imu_sync_init();*/
  servo_sync_init();
  display_sync_init();
  blackbox_sync_init();
  buzzer_sync_init();

  buzzer_quindar_tones();

  controller_state = CONTROLLER_STATE_READY;

  systime_t time = chVTGetSystemTime();
  while (true) {
    /* Fire the thread every 1 ms (1 kHz).*/
    time = chTimeAddX(time, TIME_MS2I(1));

    controller_loop();

    chThdSleepUntil(time);
  }
}

void shellcmd_controller(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  chprintf(chp, "controller state: %d\r\n", (int32_t)controller_state);
}
