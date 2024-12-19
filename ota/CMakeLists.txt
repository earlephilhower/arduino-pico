cmake_minimum_required(VERSION 3.12)

set(cpu $ENV{CPU})
message("Building for CPU ${cpu}")

if (${cpu} MATCHES "rp2040")
    set(PICO_BOARD pico)
    set(PICO_PLATFORM rp2040)
    set(PICO_CYW43_SUPPORTED 0)
    set(OBJARCH armv6-m)
    set(WRAP -Wl,--wrap=clocks_init)
    set(TUPLE arm-none-eabi)
elseif(${cpu} MATCHES "rp2350$")
    set(PICO_BOARD pico2)
    set(PICO_PLATFORM rp2350)
    set(PICO_CYW43_SUPPORTED 0)
    set(OBJARCH armv8-m)
    set(WRAP )
    set(TUPLE arm-none-eabi)
elseif(${cpu} MATCHES "rp2350-riscv$")
    set(PICO_BOARD pico2)
    set(PICO_PLATFORM rp2350-riscv)
    set(PICO_CYW43_SUPPORTED 0)
    set(OBJARCH rv32imac_zicsr_zifencei_zba_zbb_zbs_zbkb)
    set(WRAP )
    set(TUPLE riscv32-unknown-elf)
else()
    message(FATAL_ERROR "Unknown CPU, '${cpu}'")
endif()


include(pico_sdk_import.cmake)

project(pico_lib C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

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
        PICO_RUNTIME_INIT_AEABI_BIT_OPS=00090
        PICO_RUNTIME_INIT_AEABI_MEM_OPS=00091
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
        -Wall
        -Werror
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

target_link_options(ota PUBLIC
        ${WRAP}
        -Wl,--wrap=exit
        -Wl,--wrap=atexit
        -Wl,--wrap=panic_unsupported
        -Wl,--wrap=panic
        -Wl,--wrap=hard_assertion_failure
        -Wl,--cref
)

set_target_properties(ota PROPERTIES PICO_TARGET_LINKER_SCRIPT
    ${CMAKE_SOURCE_DIR}/memmap_ota_${cpu}.ld)

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
        COMMAND ../../system/${TUPLE}/bin/${TUPLE}-ld -r -A ${OBJARCH} -b binary -o ota.o ota.bin
        COMMAND ../../system/${TUPLE}/bin/${TUPLE}-objcopy --rename-section .data=.OTA ota.o ../../lib/${cpu}/ota.o
)

if(NOT ${cpu} MATCHES "rp2350-riscv$")
        # riscv build uses the same include directories in lib/rp2350-riscv/platform_inc.txt
        # so we don't copy in that case
        add_custom_command(TARGET ota POST_BUILD
                COMMAND cp ../ota_command.h ../../include/${cpu}/pico_base/pico/.
        )
endif()
