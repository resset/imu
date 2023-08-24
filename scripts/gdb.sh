#!/bin/bash

PYTHONHOME=${HOME}/.pyenv/versions/3.8.12 arm-none-eabi-gdb -x ../../../chibios/cmd.gdb -f build/imu.elf
