#!/bin/bash

export PICO_SDK_PATH=~/.pico-sdk/sdk/2.1.0

ARTIFACT="usb2ps2"

echo "PICO_SDK_PATH ha sido configurado a $PICO_SDK_PATH"
PROJECT_DIR="."
BUILD_DIR="$PROJECT_DIR/build"
if [ -d "$BUILD_DIR" ]; then
    echo "Eliminando el directorio build..."
    rm -rf "$BUILD_DIR"
    echo "Directorio build eliminado."
else
    echo "El directorio build no existe."
fi

mkdir $BUILD_DIR
cd $BUILD_DIR

cmake -Wno-dev -DCMAKE_BUILD_TYPE=Debug .. 
make
cd ..


/Users/destroyer/.pico-sdk/openocd/0.12.0+dev/openocd.exe -s /Users/destroyer/.pico-sdk/openocd/0.12.0+dev/scripts -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program build/usb2ps2.elf verify reset exit"