#!/bin/bash

set -e # Exit on error
set -x

PICO_SDK_PATH="$(cd ../../pico-sdk/; pwd)"
export PICO_SDK_PATH
PATH="$(cd ../../system/arm-none-eabi/bin; pwd):$PATH"
PATH="$(cd ../../system/riscv32-unknown-elf/bin; pwd):$PATH"
PATH="$(cd ../../system/picotool; pwd):$PATH"
export PATH

# Parse arguments
targets=()
flags=()
parse_flags=0
for arg in "$@"; do
    if [[ $parse_flags -eq 1 ]]; then
        flags+=("$arg")
    elif [[ $arg == "--flags" ]]; then
        parse_flags=1
    else
        targets+=("$arg")
    fi
done

build_rp2040() {
    rm -rf build-rp2040
    mkdir build-rp2040
    cd build-rp2040
    CPU=rp2040 cmake "${flags[@]}" ..
    make -j
# The develop branch of the SDK seems to have busted the RP2040 boot2.S files.
# These don't change and aren't lkikely to get any new additions, so comment out
# for now and use the prior versions built under earlier SDK.
#rm -rf boot
#mkdir boot
#cd boot
#mkdir -p pico
#touch pico/config.h
#for type in boot2_generic_03h boot2_is25lp080 boot2_w25q080 boot2_w25x10cl; do
#    for div in 2 4; do
#        arm-none-eabi-gcc -march=armv6-m -mcpu=cortex-m0plus -mthumb -O3 \
#                         -DNDEBUG -DPICO_FLASH_SPI_CLKDIV=$div \
#                         -c "$PICO_SDK_PATH/src/rp2040/boot_stage2/$type.S" \
#                         -I "$PICO_SDK_PATH/src/boards/include/boards/" \
#                         -I "$PICO_SDK_PATH/src/rp2040/hardware_regs/include/" \
#                         -I "$PICO_SDK_PATH/src/rp2_common/pico_platform/include/" \
#                         -I "$PICO_SDK_PATH/src/rp2_common/boot_stage2/asminclude/" \
#                         -I "$PICO_SDK_PATH/src/rp2040/pico_platform/include/" \
#                         -I "$PICO_SDK_PATH/src/rp2040/boot_stage2/asminclude/" \
#                         -I .
#
#        arm-none-eabi-gcc -march=armv6-m -mcpu=cortex-m0plus -mthumb -O3 \
#                          -DNDEBUG -Wl,--build-id=none --specs=nosys.specs -nostartfiles \
#                          -Wl,--script="$PICO_SDK_PATH/src/rp2040/boot_stage2/boot_stage2.ld" \
#                          -Wl,-Map=$type.$div.elf.map $type.o -o $type.$div.elf
#
#        arm-none-eabi-objdump -h $type.$div.elf >  $type.$div.dis
#        arm-none-eabi-objdump -d $type.$div.elf >> $type.$div.dis
#
#        arm-none-eabi-objcopy -Obinary $type.$div.elf $type.$div.bin
#
#        python3 "$PICO_SDK_PATH/src/rp2040/boot_stage2/pad_checksum" \
#                   -s 0xffffffff $type.$div.bin ${type}_${div}_padded_checksum.S
#    done
#done
#mv *.S ../../../../boot2/rp2040/.
#cd ../..
    cd ..
}

build_rp2350() {
    rm -rf build-rp2350
    mkdir build-rp2350
    cd build-rp2350
    CPU=rp2350 cmake "${flags[@]}" ..
    make -j
    cd ..
}

build_rp2350_riscv() {
    rm -rf build-rp2350-riscv
    mkdir build-rp2350-riscv
    cd build-rp2350-riscv
    CPU=rp2350-riscv cmake "${flags[@]}" ..
    make -j
  cd ..
}

if [[ ${#targets[@]} -eq 0 ]]; then
    build_rp2040
    build_rp2350
    build_rp2350_riscv
else
    for t in "${targets[@]}"; do
        case $t in
            rp2040) build_rp2040 ;;
            rp2350) build_rp2350 ;;
            rp2350-riscv) build_rp2350_riscv ;;
            *) echo "Unknown target: $t"; exit 1 ;;
        esac
    done
fi
