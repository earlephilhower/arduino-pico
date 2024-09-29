#!/bin/bash

set -e # Exit on error

export PICO_SDK_PATH="$(cd ../pico-sdk/; pwd)"
export PATH="$(cd ../system/arm-none-eabi/bin; pwd):$PATH"
export PATH="$(cd ../system/riscv32-unknown-elf/bin; pwd):$PATH"

rm -rf build
mkdir build
cd build
CPU=rp2040 cmake ..
CPU=rp2040 make -j # VERBOSE=1
cd ..

rm -rf build-rp2350
mkdir build-rp2350
cd build-rp2350
CPU=rp2350 cmake .. -DPICO_RUNTIME_SKIP_INIT_DEFAULT_ALARM_POOL=1
CPU=rp2350 make -j # VERBOSE=1
cd ..

rm -rf build-rp2350-riscv
mkdir build-rp2350-riscv
cd build-rp2350-riscv
CPU=rp2350-riscv cmake .. -DPICO_RUNTIME_SKIP_INIT_DEFAULT_ALARM_POOL=1
CPU=rp2350-riscv make -j # VERBOSE=1
cd ..
