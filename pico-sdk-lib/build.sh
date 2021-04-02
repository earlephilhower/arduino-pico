#!/bin/bash
rm -rf build
mkdir build
cd build
PICO_SDK_PATH=../../pico-sdk/ PATH="$(cd ../../system/arm-none-eabi/bin; pwd):$PATH" cmake ..
make -j32
cp libpico.a ../../lib/.
cp generated/pico_base/pico/version.h ../../pico_base/pico/.
cd ..

rm -rf boot
mkdir boot
cd boot
../boot.sh
cp *.S ../../assembly/.
cd ..

