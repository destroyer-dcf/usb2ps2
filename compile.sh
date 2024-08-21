#!/bin/bash

export PICO_SDK_PATH=~/pico-sdk
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

cmake ..
make