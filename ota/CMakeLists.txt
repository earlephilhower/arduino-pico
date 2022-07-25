cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(pico_lib C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(PICO_BOARD pico)

# Initialize the SDK
pico_sdk_init()

add_executable(ota ota.c)
pico_add_extra_outputs(ota)

# Use a longer XOSC startup time, to accommodate Adafruit and other boards that may need it.
target_compile_definitions(ota PUBLIC
	PICO_FLASH_SIZE_BYTES=16777216
	PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
        PICO_RP2040_B0_SUPPORTED=1
        PICO_RP2040_B1_SUPPORTED=1
        PICO_RP2040_B2_SUPPORTED=1
        PICO_PRINTF_SUPPORT_FLOAT=0
        PICO_PRINTF_SUPPORT_LONG_LONG=0
)

target_compile_options(ota PUBLIC
	-fno-exceptions
	-Os
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

target_link_options(ota PUBLIC
        -Wl,--wrap=exit
        -Wl,--wrap=atexit
)

target_link_libraries(ota pico_platform pico_standard_link pico_stdlib)

add_custom_command(TARGET ota POST_BUILD
        COMMAND ../../system/arm-none-eabi/bin/arm-none-eabi-ld -r -A armv6-m  -b binary -o ota.o ota.bin
        COMMAND ../../system/arm-none-eabi/bin/arm-none-eabi-objcopy --rename-section .data=.OTA ota.o ../../lib/ota.o
)
