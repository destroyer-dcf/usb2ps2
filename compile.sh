#!/bin/bash

export PICO_SDK_PATH=~/pico-sdk

ARTIFACT="usb2ps2"

echo "PICO_SDK_PATH ha sido configurado a $PICO_SDK_PATH"
PROJECT_DIR="."
BUILD_DIR="$PROJECT_DIR/build"
# if [ -d "$BUILD_DIR" ]; then
#     echo "Eliminando el directorio build..."
#     rm -rf "$BUILD_DIR"
#     echo "Directorio build eliminado."
# else
#     echo "El directorio build no existe."
# fi

mkdir $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_BUILD_TYPE=Debug .. 
make
cd ..
# cp build/usb2ps2.uf2 /media/destroyer/RPI-RP2