#!/bin/bash

export PICO_SDK_PATH=~/pico/pico-sdk
OPENOCD_PATH=/home/destroyer/Documents/openocd
OPENOCD_BIN=$OPENOCD_PATH/src/openocd

"$OPENOCD_BIN" -f $OPENOCD_PATH/tcl/interface/cmsis-dap.cfg -f $OPENOCD_PATH/tcl/target/rp2040.cfg -c "adapter speed 5000" -c "program build/usb2ps2.elf verify reset exit"
