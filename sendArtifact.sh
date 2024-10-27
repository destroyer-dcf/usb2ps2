#!/bin/bash

export PICO_SDK_PATH=~/pico-sdk

/home/destroyer/Documentos/openocd/src/openocd -f /home/destroyer/Documentos/openocd/tcl/interface/cmsis-dap.cfg -f /home/destroyer/Documentos/openocd/tcl/target/rp2040.cfg -c "adapter speed 5000" -c "program build/usb2ps2.elf verify reset exit"