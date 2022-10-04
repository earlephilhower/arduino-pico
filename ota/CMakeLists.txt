cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(pico_lib C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(PICO_BOARD pico)
set(PICO_COPY_TO_RAM 1)

# Initialize the SDK
pico_sdk_init()

add_executable(ota
        ota.c
        ota_lfs.c
        ota_clocks.c
        ../libraries/LittleFS/lib/littlefs/lfs.c
        ../libraries/LittleFS/lib/littlefs/lfs_util.c
        ./uzlib/src/tinflate.c
        ./uzlib/src/tinfgzip.c
)
pico_add_extra_outputs(ota)
pico_enable_stdio_usb(ota 0)
pico_enable_stdio_uart(ota 0)

# Use a longer XOSC startup time, to accommodate Adafruit and other boards that may need it.
target_compile_definitions(ota PUBLIC
	PICO_FLASH_SIZE_BYTES=16777216
	PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
        PICO_RP2040_B0_SUPPORTED=1
        PICO_RP2040_B1_SUPPORTED=1
        PICO_RP2040_B2_SUPPORTED=1
        PICO_PRINTF_SUPPORT_FLOAT=0
        PICO_PRINTF_SUPPORT_LONG_LONG=0
        LIB_PICO_PRINTF_NONE=1
        LFS_READONLY=1
        LFS_NO_DEBUG=1
        LFS_NO_WARN=1
        LFS_NO_ERROR=1
        LFS_NO_ASSERT=1
        LFS_NO_MALLOC=1
        PICO_PANIC_FUNCTION=
        PICO_TIME_DEFAULT_ALARM_POOL_DISABLED=1
        PICO_NO_BINARY_INFO=1
)

target_compile_options(ota PUBLIC
	-fno-exceptions
	-Os
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

target_link_options(ota PUBLIC
        -Wl,--wrap=clocks_init
        -Wl,--wrap=exit
        -Wl,--wrap=atexit
        -Wl,--wrap=panic_unsupported
        -Wl,--wrap=panic
        -Wl,--wrap=hard_assertion_failure
        -Wl,--cref
)

set_target_properties(ota PROPERTIES PICO_TARGET_LINKER_SCRIPT
    ${CMAKE_SOURCE_DIR}/memmap_ota.ld)

target_link_libraries(ota
        pico_platform
        pico_standard_link
        hardware_irq
        hardware_flash
        pico_time
        hardware_gpio
        hardware_uart
        hardware_resets
        hardware_clocks
        pico_stdlib
)

add_custom_command(TARGET ota POST_BUILD
        COMMAND ../../system/arm-none-eabi/bin/arm-none-eabi-ld -r -A armv6-m  -b binary -o ota.o ota.bin
        COMMAND ../../system/arm-none-eabi/bin/arm-none-eabi-objcopy --rename-section .data=.OTA ota.o ../../lib/ota.o
        COMMAND cp ../ota_command.h ../../include/pico_base/pico/.
)
