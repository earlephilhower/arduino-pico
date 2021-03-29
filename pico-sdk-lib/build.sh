#!/bin/bash

rm -rf build
mkdir build
cd build
PICO_SDK_PATH=../../pico-sdk/ PATH="$(cd ../../system/arm-none-eabi/bin; pwd):$PATH" cmake ..
make
cp libpico.a ../../lib/.

cd ..
rm -rf build_b1
mkdir build_b1
cd build_b1
PICO_SDK_PATH=../../pico-sdk/ PATH="$(cd ../../system/arm-none-eabi/bin; pwd):$PATH" cmake .. -DPICO_BOARD=adafruit_feather_rp2040
make
cp libpico.a ../../lib/libpico_b1.a
