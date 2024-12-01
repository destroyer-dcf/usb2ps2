#!/bin/bash

export PICO_SDK_PATH=~/pico-sdk



# cp build/usb2ps2.uf2 /media/destroyer/RPI-RP2
minicom -b 115200 -o -D /dev/ttyACM0
#/home/destroyer/Documentos/openocd/src/openocd -f /home/destroyer/Documentos/openocd/tcl/interface/cmsis-dap.cfg -f /home/destroyer/Documentos/openocd/tcl/target/rp2040.cfg -c "adapter speed 5000"
