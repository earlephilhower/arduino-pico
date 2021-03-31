mkdir -p pico 
touch pico/config.h

for type in boot2_generic_03h boot2_is25lp080 boot2_w25q080 boot2_w25x10cl; do
	for div in 2 4; do
		../../system/arm-none-eabi/bin/arm-none-eabi-gcc -march=armv6-m -mcpu=cortex-m0plus -mthumb -O3 -DNDEBUG -DPICO_FLASH_SPI_CLKDIV=$div -c ../../pico-sdk/src/rp2_common/boot_stage2/$type.S -I ../../pico-sdk/src/rp2_common/pico_platform/include/ -I ../../pico-sdk/src/rp2040/hardware_regs/include/ -I../../pico-sdk/src/rp2_common/boot_stage2/asminclude/ -I.

		../../system/arm-none-eabi/bin/arm-none-eabi-gcc -march=armv6-m -mcpu=cortex-m0plus -mthumb -O3 -DNDEBUG -Wl,--build-id=none --specs=nosys.specs -nostartfiles -Wl,--script=../../pico-sdk/src/rp2_common/boot_stage2/boot_stage2.ld -Wl,-Map=$type.$div.elf.map $type.o -o $type.$div.elf 

		../../system/arm-none-eabi/bin/arm-none-eabi-objdump -h $type.$div.elf >  $type.$div.dis
		../../system/arm-none-eabi/bin/arm-none-eabi-objdump -d $type.$div.elf >> $type.$div.dis

		../../system/arm-none-eabi/bin/arm-none-eabi-objcopy -Obinary $type.$div.elf $type.$div.bin

		python3 ../../pico-sdk/src/rp2_common/boot_stage2/pad_checksum -s 0xffffffff $type.$div.bin ${type}_${div}_padded_checksum.S
	done
done

