#!/bin/bash
rm -rf build
mkdir build
cd build
PICO_SDK_PATH=../../pico-sdk/ PATH="$(cd ../../system/arm-none-eabi/bin; pwd):$PATH" cmake ..
make
cp libpico.a ../../lib/.
cp generated/pico_base/pico/version.h ../../pico_base/pico/.
