#!/bin/bash

set -e # Exit on error
set -x

export PICO_SDK_PATH="$(cd ../../pico-sdk/; pwd)"
export PATH="$(cd ../../system/arm-none-eabi/bin; pwd):$PATH"

rm -rf build
mkdir build
cd build
cmake ..
make -j

# Put everything in its place
#cp ../lwipopts.h ../../../include/.
#mv generated/pico_base/pico/version.h ../../../include/rp2040/pico_base/pico/.
#cp ../tusb_config.h ../../../include/rp2040/.
#cp ../btstack_config.h ../../../include/rp2040/.

rm -rf boot
mkdir boot
cd boot
mkdir -p pico 
touch pico/config.h
for type in boot2_generic_03h boot2_is25lp080 boot2_w25q080 boot2_w25x10cl; do
    for div in 2 4; do
        arm-none-eabi-gcc -march=armv6-m -mcpu=cortex-m0plus -mthumb -O3 \
                         -DNDEBUG -DPICO_FLASH_SPI_CLKDIV=$div \
                         -c "$PICO_SDK_PATH/src/rp2040/boot_stage2/$type.S" \
                         -I "$PICO_SDK_PATH/src/boards/include/boards/" \
                         -I "$PICO_SDK_PATH/src/rp2040/hardware_regs/include/" \
                         -I "$PICO_SDK_PATH/src/rp2_common/pico_platform/include/" \
                         -I "$PICO_SDK_PATH/src/rp2_common/boot_stage2/asminclude/" \
                         -I "$PICO_SDK_PATH/src/rp2040/pico_platform/include/" \
                         -I "$PICO_SDK_PATH/src/rp2040/boot_stage2/asminclude/" \
                         -I .

        arm-none-eabi-gcc -march=armv6-m -mcpu=cortex-m0plus -mthumb -O3 \
                          -DNDEBUG -Wl,--build-id=none --specs=nosys.specs -nostartfiles \
                          -Wl,--script="$PICO_SDK_PATH/src/rp2040/boot_stage2/boot_stage2.ld" \
                          -Wl,-Map=$type.$div.elf.map $type.o -o $type.$div.elf 

        arm-none-eabi-objdump -h $type.$div.elf >  $type.$div.dis
        arm-none-eabi-objdump -d $type.$div.elf >> $type.$div.dis

        arm-none-eabi-objcopy -Obinary $type.$div.elf $type.$div.bin

        python3 "$PICO_SDK_PATH/src/rp2040/boot_stage2/pad_checksum" \
                   -s 0xffffffff $type.$div.bin ${type}_${div}_padded_checksum.S
    done
done
mv *.S ../../../../boot2/rp2040/.

#for type in boot2_at25sf128a boot2_generic_03h boot2_is25lp080 boot2_w25q080 boot2_w25x10cl; do
for type in boot2_generic_03h boot2_w25q080; do
    for div in 2 4; do
        arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -march=armv8-m.main+fp+dsp -mfloat-abi=softfp -mcmse -O3 \
                         -DNDEBUG -DPICO_FLASH_SPI_CLKDIV=$div \
                         -DPICO_RP2350 \
                         -c "$PICO_SDK_PATH/src/rp2350/boot_stage2/$type.S" \
                         -I "$PICO_SDK_PATH/src/boards/include/boards/" \
                         -I "$PICO_SDK_PATH/src/rp2350/hardware_regs/include/" \
                         -I "$PICO_SDK_PATH/src/rp2040/hardware_regs/include/" \
                         -I "$PICO_SDK_PATH/src/rp2_common/pico_platform/include/" \
                         -I "$PICO_SDK_PATH/src/rp2_common/boot_stage2/asminclude/" \
                         -I "$PICO_SDK_PATH/src/rp2350/pico_platform/include/" \
                         -I "$PICO_SDK_PATH/src/rp2350/boot_stage2/asminclude/" \
                         -I .

        arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -march=armv8-m.main+fp+dsp -mfloat-abi=softfp -mcmse -O3 \
                          -DNDEBUG -Wl,--build-id=none --specs=nosys.specs -nostartfiles \
                          -Wl,--script="$PICO_SDK_PATH/src/rp2350/boot_stage2/boot_stage2.ld" \
                          -Wl,-Map=$type.$div.elf.map $type.o -o $type.$div.elf

        arm-none-eabi-objdump -h $type.$div.elf >  $type.$div.dis
        arm-none-eabi-objdump -d $type.$div.elf >> $type.$div.dis

        arm-none-eabi-objcopy -Obinary $type.$div.elf $type.$div.bin

        python3 "$PICO_SDK_PATH/src/rp2350/boot_stage2/pad_checksum" \
                   -s 0xffffffff $type.$div.bin ${type}_${div}_padded_checksum.S
    done
done
mv *.S ../../../../boot2/rp2350/.
