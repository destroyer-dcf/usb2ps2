#!/bin/bash

export PICO_SDK_PATH=~/pico/pico-sdk

/home/destroyer/Documents/openocd/src/openocd -f /home/destroyer/Documents/openocd/tcl/interface/cmsis-dap.cfg -f /home/destroyer/Documents/openocd/tcl/target/rp2040.cfg -c "adapter speed 5000" -c "program build/usb2ps2.elf verify reset exit"
