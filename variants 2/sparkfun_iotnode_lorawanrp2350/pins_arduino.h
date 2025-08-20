#pragma once

#define PICO_RP2350A 1

// LEDs
#define PIN_LED (25u)

#define PIN_NEOPIXEL (25)
#define NUM_NEOPIXEL (1)

// UARTs
#define PIN_SERIAL1_TX (18u)
#define PIN_SERIAL1_RX (19u)
#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
#define PIN_SPI0_MISO (12u)
#define PIN_SPI0_MOSI (15u)
#define PIN_SPI0_SCK (14u)
#define PIN_SPI0_SS (13u)
#define PIN_SPI1_MISO (31u)
#define PIN_SPI1_MOSI (31u)
#define PIN_SPI1_SCK (31u)
#define PIN_SPI1_SS (31u) // not pinned out

// The board uses SPI1 for uSD card, make that the default
#ifndef __SPI0_DEVICE
#define __SPI0_DEVICE spi1
#endif
#ifndef __SPI1_DEVICE
#define __SPI1_DEVICE spi0
#endif

// Wire
#define PIN_WIRE0_SDA (20u)
#define PIN_WIRE0_SCL (21u)
#define PIN_WIRE1_SDA (31u)
#define PIN_WIRE1_SCL (31u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY (1u)
#define WIRE_HOWMANY (1u)

// PSRAM
#define RP2350_PSRAM_CS (0u)
#define RP2350_PSRAM_MAX_SCK_HZ (109 * 1000 * 1000)

#include "../generic/common.h"
