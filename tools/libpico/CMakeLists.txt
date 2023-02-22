cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(pico_lib C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# Enable PicoW driver support.  Compatible with standard Pico
set(PICO_BOARD pico_w)

# Initialize the SDK
pico_sdk_init()

add_library(common INTERFACE)

# Use a longer XOSC startup time, to accommodate Adafruit and other boards that may need it.
target_compile_definitions(common INTERFACE
	PICO_PRINTF_ALWAYS_INCLUDED=1
	PICO_FLASH_SIZE_BYTES=16777216
	PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
        CYW43_WARN=//
        PICO_FLOAT_SUPPORT_ROM_V1=1
        PICO_DOUBLE_SUPPORT_ROM_V1=1
        PICO_RP2040_B0_SUPPORTED=1
        PICO_RP2040_B1_SUPPORTED=1
        PICO_RP2040_B2_SUPPORTED=1
        PICO_NO_BINARY_INFO=1
        PICO_CYW43_ARCH_THREADSAFE_BACKGROUND=1
        LWIP_IPV4=1
        LWIP_UDP=1
        LWIP_IGMP=1
        LWIP_CHECKSUM_CTRL_PER_NETIF=1
)

target_compile_options(common INTERFACE
	-fno-exceptions
	-Os
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

include_directories(BEFORE ${PICO_SDK_PATH}/../tools/libpico)

add_library(pico STATIC)
target_compile_definitions(pico PUBLIC
        LWIP_IPV6=0
)

target_link_libraries(pico
        common
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
	pico_standard_link
	pico_stdlib
	pico_unique_id
	pico_util
	tinyusb
	tinyusb_device_unmarked
)

add_library(picow-noipv6-nobtc-noble STATIC)
target_compile_definitions(picow-noipv6-nobtc-noble PUBLIC
        LWIP_IPV6=0
)

target_link_libraries(picow-noipv6-nobtc-noble
        common
	pico_stdlib
        pico_lwip_sntp
	cyw43_driver
	cyw43_driver_picow
	pico_async_context
	pico_cyw43_driver
	pico_cyw43_arch
	pico_cyw43_arch_threadsafe_background
	pico_lwip
	pico_lwip_nosys
        pico_lwip_sntp
)

add_library(picow-ipv6-nobtc-noble STATIC)
target_compile_definitions(picow-ipv6-nobtc-noble PUBLIC
        LWIP_IPV6=1
)

target_link_libraries(picow-ipv6-nobtc-noble
        common
	pico_stdlib
        pico_lwip_sntp
        cyw43_driver
        cyw43_driver_picow
        pico_async_context
        pico_cyw43_driver
        pico_cyw43_arch
        pico_cyw43_arch_threadsafe_background
        pico_lwip
        pico_lwip_nosys
)

# Don't build all 8 combinations, just all-on or off for BT
if (FALSE)
add_library(picow-noipv6-btc-noble STATIC)
target_compile_definitions(picow-noipv6-btc-noble PUBLIC
        LWIP_IPV6=0
)

target_link_libraries(picow-noipv6-btc-noble
        common
        pico_stdlib
        pico_lwip_sntp
        cyw43_driver
        cyw43_driver_picow
        pico_async_context
        pico_cyw43_driver
        pico_cyw43_arch
        pico_cyw43_arch_threadsafe_background
        pico_lwip
        pico_lwip_nosys
        pico_lwip_sntp
        pico_btstack_cyw43
        pico_btstack_classic
)

add_library(picow-ipv6-btc-noble STATIC)
target_compile_definitions(picow-ipv6-btc-noble PUBLIC
        LWIP_IPV6=1
)

target_link_libraries(picow-ipv6-btc-noble
        common
        pico_stdlib
        pico_lwip_sntp
        cyw43_driver
        cyw43_driver_picow
        pico_async_context
        pico_cyw43_driver
        pico_cyw43_arch
        pico_cyw43_arch_threadsafe_background
        pico_lwip
        pico_lwip_nosys
        pico_btstack_cyw43
        pico_btstack_classic
)

add_library(picow-noipv6-nobtc-ble STATIC)
target_compile_definitions(picow-noipv6-nobtc-ble PUBLIC
        LWIP_IPV6=0
)

target_link_libraries(picow-noipv6-nobtc-ble
        common
        pico_stdlib
        pico_lwip_sntp
        cyw43_driver
        cyw43_driver_picow
        pico_async_context
        pico_cyw43_driver
        pico_cyw43_arch
        pico_cyw43_arch_threadsafe_background
        pico_lwip
        pico_lwip_nosys
        pico_lwip_sntp
        pico_btstack_cyw43
        pico_btstack_ble
)

add_library(picow-ipv6-nobtc-ble STATIC)
target_compile_definitions(picow-ipv6-nobtc-ble PUBLIC
        LWIP_IPV6=1
)

target_link_libraries(picow-ipv6-nobtc-ble
        common
        pico_stdlib
        pico_lwip_sntp
        cyw43_driver
        cyw43_driver_picow
        pico_async_context
        pico_cyw43_driver
        pico_cyw43_arch
        pico_cyw43_arch_threadsafe_background
        pico_lwip
        pico_lwip_nosys
        pico_btstack_cyw43
        pico_btstack_ble
)
endif()

add_library(picow-noipv6-btc-ble STATIC)
target_compile_definitions(picow-noipv6-btc-ble PUBLIC
        LWIP_IPV6=0
)

target_link_libraries(picow-noipv6-btc-ble
        common
        pico_stdlib
        pico_lwip_sntp
        cyw43_driver
        cyw43_driver_picow
        pico_async_context
        pico_cyw43_driver
        pico_cyw43_arch
        pico_cyw43_arch_threadsafe_background
        pico_lwip
        pico_lwip_nosys
        pico_lwip_sntp
        pico_btstack_cyw43
        pico_btstack_ble
        pico_btstack_classic
)

add_library(picow-ipv6-btc-ble STATIC)
target_compile_definitions(picow-ipv6-btc-ble PUBLIC
        LWIP_IPV6=1
)

target_link_libraries(picow-ipv6-btc-ble
        common
        pico_stdlib
        pico_lwip_sntp
        cyw43_driver
        cyw43_driver_picow
        pico_async_context
        pico_cyw43_driver
        pico_cyw43_arch
        pico_cyw43_arch_threadsafe_background
        pico_lwip
        pico_lwip_nosys
        pico_btstack_cyw43
        pico_btstack_ble
        pico_btstack_classic
)

add_custom_command(TARGET pico POST_BUILD
	COMMAND ar d libpico.a stdio.c.obj stdio_usb.c.obj stdio_usb_descriptors.c.obj pico_malloc.c.obj
)

add_custom_command(TARGET picow-noipv6-nobtc-noble POST_BUILD
	COMMAND ar d libpicow-noipv6-nobtc-noble.a stdio.c.obj stdio_usb.c.obj stdio_usb_descriptors.c.obj pico_malloc.c.obj
)

add_custom_command(TARGET picow-ipv6-nobtc-noble POST_BUILD
	COMMAND ar d libpicow-ipv6-nobtc-noble.a stdio.c.obj stdio_usb.c.obj stdio_usb_descriptors.c.obj pico_malloc.c.obj
)

if (FALSE)
add_custom_command(TARGET picow-noipv6-btc-noble POST_BUILD
	COMMAND ar d libpicow-noipv6-btc-noble.a stdio.c.obj stdio_usb.c.obj stdio_usb_descriptors.c.obj pico_malloc.c.obj
)

add_custom_command(TARGET picow-ipv6-btc-noble POST_BUILD
	COMMAND ar d libpicow-ipv6-btc-noble.a stdio.c.obj stdio_usb.c.obj stdio_usb_descriptors.c.obj pico_malloc.c.obj
)

add_custom_command(TARGET picow-noipv6-nobtc-ble POST_BUILD
	COMMAND ar d libpicow-noipv6-nobtc-ble.a stdio.c.obj stdio_usb.c.obj stdio_usb_descriptors.c.obj pico_malloc.c.obj
)

add_custom_command(TARGET picow-ipv6-nobtc-ble POST_BUILD
	COMMAND ar d libpicow-ipv6-nobtc-ble.a stdio.c.obj stdio_usb.c.obj stdio_usb_descriptors.c.obj pico_malloc.c.obj
)
endif()

add_custom_command(TARGET picow-noipv6-btc-ble POST_BUILD
        COMMAND ar d libpicow-noipv6-btc-ble.a stdio.c.obj stdio_usb.c.obj stdio_usb_descriptors.c.obj pico_malloc.c.obj
)

add_custom_command(TARGET picow-ipv6-btc-ble POST_BUILD
        COMMAND ar d libpicow-ipv6-btc-ble.a stdio.c.obj stdio_usb.c.obj stdio_usb_descriptors.c.obj pico_malloc.c.obj
)
