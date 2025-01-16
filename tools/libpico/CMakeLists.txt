cmake_minimum_required(VERSION 3.12)

set(cpu $ENV{CPU})
message("Building for CPU ${cpu}")

set(PICO_CYW43_SUPPORTED 1)
if (${cpu} MATCHES "rp2040")
    # Enable PicoW driver support.  Compatible with standard Pico
    set(PICO_BOARD pico_w)
    set(PICO_PLATFORM rp2040)
elseif(${cpu} MATCHES "rp2350$")
    set(PICO_BOARD solderparty_rp2350_stamp_xl)  # Pico2 sets to RP2350A which disables all code for RP2350B
    set(PICO_PLATFORM rp2350)
elseif(${cpu} MATCHES "rp2350-riscv$")
    message(INFO "Building RISCV")
    set(PICO_BOARD solderparty_rp2350_stamp_xl)  # Pico2 sets to RP2350A which disables all code for RP2350B
    set(PICO_PLATFORM rp2350-riscv)
else()
    message(FATAL_ERROR "Unknown CPU, '${cpu}'")
endif()

include(pico_sdk_import.cmake)

project(pico_lib C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# Initialize the SDK
pico_sdk_init()

add_library(common-${cpu} INTERFACE)

if (${cpu} MATCHES "rp2350$")
    set(xcda
        LIB_PICO_DOUBLE_PICO=1
        LIB_PICO_FLOAT_PICO=1
        LIB_PICO_FLOAT_PICO_VFP=1)
elseif (${cpu} MATCHES "rp2350-riscv$")
    set(xcda
        LIB_PICO_DOUBLE_COMPILER=1
        LIB_PICO_FLOAT_COMPILER=1
        PICO_RISCV=1)
endif()

if (${cpu} MATCHES "rp2040")
    set(xcd
        PICO_RP2040_B0_SUPPORTED=1
        PICO_RP2040_B1_SUPPORTED=1
        PICO_RP2040_B2_SUPPORTED=1
        PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
        PICO_FLOAT_SUPPORT_ROM_V1=1
        PICO_DOUBLE_SUPPORT_ROM_V1=1
        PICO_RP2040=1)
elseif (${cpu} MATCHES "rp2350")
    set(xcd
        CFG_TUSB_DEBUG=0
        CFG_TUSB_MCU=OPT_MCU_RP2040
        CFG_TUSB_OS=OPT_OS_PICO
        LIB_BOOT_STAGE2_HEADERS=1
        LIB_PICO_ATOMIC=1
        LIB_PICO_BIT_OPS=1
        LIB_PICO_BIT_OPS_PICO=1
        LIB_PICO_CLIB_INTERFACE=1
        LIB_PICO_CRT0=1
        LIB_PICO_CXX_OPTIONS=1
        LIB_PICO_DIVIDER=1
        LIB_PICO_DIVIDER_COMPILER=1
        LIB_PICO_DOUBLE=1
        LIB_PICO_FIX_RP2040_USB_DEVICE_ENUMERATION=1
        LIB_PICO_FLOAT=1
        LIB_PICO_INT64_OPS=1
        LIB_PICO_INT64_OPS_COMPILER=1
        LIB_PICO_MEM_OPS=1
        LIB_PICO_MEM_OPS_COMPILER=1
        LIB_PICO_NEWLIB_INTERFACE=1
        LIB_PICO_PLATFORM=1
        LIB_PICO_PLATFORM_COMPILER=1
        LIB_PICO_PLATFORM_PANIC=1
        LIB_PICO_PLATFORM_SECTIONS=1
        LIB_PICO_RUNTIME=1
        LIB_PICO_RUNTIME_INIT=1
        LIB_PICO_STANDARD_BINARY_INFO=1
        LIB_PICO_STANDARD_LINK=1
        LIB_PICO_SYNC=1
        LIB_PICO_SYNC_CRITICAL_SECTION=1
        LIB_PICO_SYNC_MUTEX=1
        LIB_PICO_SYNC_SEM=1
        LIB_PICO_TIME=1
        LIB_PICO_TIME_ADAPTER=1
        LIB_PICO_UNIQUE_ID=1
        LIB_PICO_UTIL=1
        LIB_TINYUSB_BOARD=1
        LIB_TINYUSB_DEVICE=1
        PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
        PICO_32BIT=1
        PICO_BOARD=\"solderparty_rp2350_stamp_xl\"
        PICO_BUILD=1
        PICO_COPY_TO_RAM=0
        PICO_CXX_ENABLE_EXCEPTIONS=0
        PICO_NO_FLASH=0
        PICO_NO_HARDWARE=0
        PICO_ON_DEVICE=1
        PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1
        PICO_RP2040_USB_DEVICE_UFRAME_FIX=1
        PICO_RP2350=1
        PICO_USE_BLOCKED_RAM=0

        CYW43_DEFAULT_PIN_WL_REG_ON=23u
        CYW43_DEFAULT_PIN_WL_DATA_OUT=24u
        CYW43_DEFAULT_PIN_WL_DATA_IN=24u
        CYW43_DEFAULT_PIN_WL_HOST_WAKE=24u
        CYW43_DEFAULT_PIN_WL_CLOCK=29u
        CYW43_DEFAULT_PIN_WL_CS=25u

        ${xcda})
endif()

# Use a longer XOSC startup time, to accommodate Adafruit and other boards that may need it.
target_compile_definitions(common-${cpu} INTERFACE
    PICO_PLATFORM=${PICO_PLATFORM}
    PICO_PRINTF_ALWAYS_INCLUDED=1
    PICO_FLASH_SIZE_BYTES=16777216
    PICO_NO_BINARY_INFO=1
    LWIP_IPV4=1
    LWIP_UDP=1
    LWIP_IGMP=1
    LWIP_CHECKSUM_CTRL_PER_NETIF=1
    PICO_CYW43_SUPPORTED=1
    PICO_CYW43_ARCH_THREADSAFE_BACKGROUND=1
    CYW43_WARN=//
    CYW43_PIO_CLOCK_DIV_DYNAMIC=1
    CYW43_PIN_WL_DYNAMIC=1
    ${xcd}
)

target_compile_options(common-${cpu} INTERFACE
    -fno-exceptions
    -Os
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

include_directories(BEFORE ${PICO_SDK_PATH}/../tools/libpico)

add_library(pico-${cpu} STATIC)
target_compile_definitions(pico-${cpu} PUBLIC
    LWIP_IPV6=0
)

target_link_libraries(pico-${cpu}
    common-${cpu}
    boot_stage2
    hardware_adc
    hardware_base
    hardware_claim
    hardware_clocks
    hardware_divider
    hardware_dma
    hardware_exception
    hardware_flash
    hardware_gpio
    hardware_i2c
    hardware_interp
    hardware_irq
    hardware_pio
    hardware_pll
    hardware_pwm
    hardware_resets
    hardware_rtc
    hardware_spi
    hardware_sync
    hardware_timer
    hardware_uart
    hardware_vreg
    hardware_watchdog
    hardware_xosc
    cmsis
    pico_aon_timer
    pico_bit_ops
    pico_bootrom
    pico_bootsel_via_double_reset
    pico_cxx_options
    pico_divider
    pico_double
    pico_fix
    pico_float
    pico_int64_ops
    pico_malloc
    pico_mem_ops
    pico_multicore
    pico_platform
    pico_rand
    pico_runtime
    pico_runtime_init
    pico_standard_link
    pico_stdlib
    pico_unique_id
    pico_util
    tinyusb
    tinyusb_device_unmarked
)
if (${cpu} MATCHES "rp2350")
    # RP2350-only peripherals
    target_link_libraries(pico-${cpu}
        hardware_xip_cache
        pico_sha256)
endif()

add_library(ipv4-${cpu} STATIC)
target_compile_definitions(ipv4-${cpu} PUBLIC
    LWIP_IPV6=0
)

add_library(ipv4-big-${cpu} STATIC)
target_compile_definitions(ipv4-big-${cpu} PUBLIC
    __LWIP_MEMMULT=2
    LWIP_IPV6=0
)

set(picow_link_libraries
    common-${cpu}
    cyw43_driver
    cyw43_driver_picow
    pico_async_context
    pico_async_context_threadsafe_background
    pico_cyw43_driver
    pico_cyw43_arch
    pico_cyw43_arch_threadsafe_background
    pico_lwip
    pico_lwip_nosys
    pico_lwip_sntp
    pico_lwip_mdns
    pico_stdlib
)

target_link_libraries(ipv4-${cpu}
    ${picow_link_libraries}
)

target_link_libraries(ipv4-big-${cpu}
    ${picow_link_libraries}
)

add_library(ipv4-ipv6-${cpu} STATIC)
target_compile_definitions(ipv4-ipv6-${cpu} PUBLIC
    LWIP_IPV6=1
)

add_library(ipv4-ipv6-big-${cpu} STATIC)
target_compile_definitions(ipv4-ipv6-big-${cpu} PUBLIC
    __LWIP_MEMMULT=2
    LWIP_IPV6=1
)

target_link_libraries(ipv4-ipv6-${cpu}
    ${picow_link_libraries}
)
  
target_link_libraries(ipv4-ipv6-big-${cpu}
    ${picow_link_libraries}
)

add_library(ipv4-bt-${cpu} STATIC)
target_compile_definitions(ipv4-bt-${cpu} PUBLIC
    LWIP_IPV6=0
)

add_library(ipv4-bt-big-${cpu} STATIC)
target_compile_definitions(ipv4-bt-${cpu} PUBLIC
    __LWIP_MEMMULT=2
    LWIP_IPV6=0
)

set(picow_bt_link_libraries
    pico_btstack_cyw43
    pico_btstack_ble
    pico_btstack_classic
    pico_btstack_sbc_encoder
    pico_btstack_sbc_decoder
)

target_link_libraries(ipv4-bt-${cpu}
    ${picow_link_libraries}
    ${picow_bt_link_libraries}
)

target_link_libraries(ipv4-bt-big-${cpu}
    ${picow_link_libraries}
    ${picow_bt_link_libraries}
)

add_library(ipv4-ipv6-bt-${cpu} STATIC)
target_compile_definitions(ipv4-ipv6-bt-${cpu} PUBLIC
    LWIP_IPV6=1
)

add_library(ipv4-ipv6-bt-big-${cpu} STATIC)
target_compile_definitions(ipv4-ipv6-bt-big-${cpu} PUBLIC
    __LWIP_MEMMULT=2
    LWIP_IPV6=1
)

target_link_libraries(ipv4-ipv6-bt-${cpu}
    ${picow_link_libraries}
    ${picow_bt_link_libraries}
)

target_link_libraries(ipv4-ipv6-bt-big-${cpu}
    ${picow_link_libraries}
    ${picow_bt_link_libraries}
)

foreach(tgt ipv4 ipv4-ipv6 ipv4-big ipv4-ipv6-big ipv4-bt ipv4-ipv6-bt ipv4-bt-big ipv4-ipv6-bt-big)
    add_custom_command(TARGET ${tgt}-${cpu} POST_BUILD
        COMMAND arm-none-eabi-ar d lib${tgt}-${cpu}.a atomic.c.o bit_ops_aeabi.S.o boot_lock.c.o bootrom.c.o bootrom_lock.c.o claim.c.o clocks.c.o critical_section.c.o crt0.S.o datetime.c.o divider_hardware.S.o divider.S.o dma.c.o double_aeabi_rp2040.S.o double_init_rom_rp2040.c.o double_math.c.o double_v1_rom_shim_rp2040.S.o exception.c.o flash.c.o flash.c.o float_aeabi_rp2040.S.o float_init_rom_rp2040.c.o float_math.c.o float_v1_rom_shim_rp2040.S.o gpio.c.o irq.c.o irq_handler_chain.S.o lock_core.c.o malloc.c.o mem_ops_aeabi.S.o mutex.c.o panic.c.o pheap.c.o pico_int64_ops_aeabi.S.o pio.c.o platform.c.o pll.c.o printf.c.o queue.c.o rand.c.o runtime.c.o runtime_init_clocks.c.o runtime_init.c.o runtime_init_stack_guard.c.o sem.c.o standard_binary_info.c.o sync.c.o sync_spin_lock.c.o ticks.c.o time.c.o timeout_helper.c.o timer.c.o uart.c.o unique_id.c.o vreg.c.o watchdog.c.o xip_cache.c.o xosc.c.o
    )
endforeach()
foreach(tgt pico ipv4 ipv4-ipv6 ipv4-big ipv4-ipv6-big ipv4-bt ipv4-ipv6-bt ipv4-bt-big ipv4-ipv6-bt-big)
    add_custom_command(TARGET ${tgt}-${cpu} POST_BUILD
        COMMAND arm-none-eabi-ar d lib${tgt}-${cpu}.a pico_malloc.c.o newlib_interface.c.o stdlib.c.o stdio.c.o stdio_uart.c.o new_delete.cpp.o
        COMMAND arm-none-eabi-ar d lib${tgt}-${cpu}.a btstack_flash_bank.c.o  # Need to override with our own implementation
        COMMAND cp lib${tgt}-${cpu}.a ../../../lib/${cpu}/lib${tgt}.a
    )
endforeach()

add_custom_command(TARGET pico-${cpu} POST_BUILD
    COMMAND mkdir -p ../../../include/${cpu}/pico_base/pico
    COMMAND cp ./generated/pico_base/pico/version.h ../../../include/${cpu}/pico_base/pico/.
    COMMAND sed 's/include.*pico-sdk/include \"..\\/..\\/pico-sdk/' ./generated/pico_base/pico/config_autogen.h > ../../../include/${cpu}/pico_base/pico/config_autogen.h
    COMMAND cp ../tusb_config.h ../../../include/${cpu}/.
    COMMAND cp ../btstack_config.h ../../../include/${cpu}/.
    COMMAND cp ../lwipopts.h ../../../include/.
)
