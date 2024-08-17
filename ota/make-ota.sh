#!/bin/bash

set -e # Exit on error

export PICO_SDK_PATH="$(cd ../pico-sdk/; pwd)"
export PATH="$(cd ../system/arm-none-eabi/bin; pwd):$PATH"

rm -rf build
mkdir build
cd build
cmake ..
make -j # VERBOSE=1


# TODO - Dummy OTA for RP2350
echo "void __dummyota() {}" > dummyota.c
arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -march=armv8-m.main+fp+dsp -mfloat-abi=softfp -mcmse -c dummyota.c -o ../../lib/rp2350/ota.o
