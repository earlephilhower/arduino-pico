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

add_library(pico STATIC)

# Use a longer XOSC startup time, to accommodate Adafruit and other boards that may need it.
target_compile_definitions(pico PUBLIC
	PICO_PRINTF_ALWAYS_INCLUDED=1
	PICO_FLASH_SIZE_BYTES=16777216
	PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
        LWIP_IPV4=1
        LWIP_IPV6=${IPV6}
        LWIP_UDP=1
        LWIP_IGMP=1
        LWIP_CHECKSUM_CTRL_PER_NETIF=1
        CYW43_WARN=//
        PICO_FLOAT_SUPPORT_ROM_V1=1
        PICO_DOUBLE_SUPPORT_ROM_V1=1
        PICO_RP2040_B0_SUPPORTED=1
        PICO_RP2040_B1_SUPPORTED=1
        PICO_RP2040_B2_SUPPORTED=1
        PICO_NO_BINARY_INFO=1
)

target_compile_options(pico PUBLIC
	-fno-exceptions
	-Os
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

include_directories(BEFORE ${PICO_SDK_PATH}/../tools/libpico)

target_link_libraries(pico
	boot_stage2
	cyw43_driver
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
	pico_cyw43_arch_none
	pico_divider
	pico_double
	pico_fix
	pico_float
	pico_int64_ops
	pico_lwip
	pico_lwip_nosys
        pico_lwip_sntp
	pico_malloc
	pico_mem_ops
	pico_multicore
	pico_platform
	pico_runtime
	pico_standard_link
	pico_stdlib
	pico_unique_id
	pico_util
	tinyusb
	tinyusb_device_unmarked
)

add_custom_command(TARGET pico POST_BUILD
	COMMAND ar d libpico.a stdio.c.obj stdio_usb.c.obj stdio_usb_descriptors.c.obj cyw43_arch_threadsafe_background.c.obj
	COMMAND ar q libpico.a pico-sdk/src/rp2_common/cyw43_driver/cyw43_resource.o
)
