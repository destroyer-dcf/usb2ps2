#!/bin/bash

export PICO_SDK_PATH=~/.pico-sdk/sdk/2.1.0
# minicom -b 115200 -o -D /dev/ttyACM0

# export PICO_SDK_PATH=~/pico/pico-sdk
# export PICO_SDK_PATH=/Users/destroyer/PROJECTS/amstrad-keyboard-pico/pico-sdk
#minicom -b 115200 -o -D /dev/ttyACM0
minicom -b 115200 -o -D /dev/cu.usbmodem83302