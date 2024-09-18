#pragma once

// LEDs
#define PIN_LED        (7u)

#define PIN_NEOPIXEL   (21)
#define NUM_NEOPIXEL   (1)

// UARTs
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)
#define PIN_SERIAL2_TX (31u) // not pinned out
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI0_MISO  (20u)
#define PIN_SPI0_MOSI  (23u)
#define PIN_SPI0_SCK   (22u)
#define PIN_SPI0_SS    (21u)
#define PIN_SPI1_MISO  (31u) // not pinned out
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)

// Wire
#define PIN_WIRE0_SDA  (2u)
#define PIN_WIRE0_SCL  (3u)
#define PIN_WIRE1_SDA  (31u) // not pinned out
#define PIN_WIRE1_SCL  (31u)

#define SERIAL_HOWMANY (1u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (1u)

// PSRAM
#define RP2350_PSRAM_CS         (8u)
#define RP2350_PSRAM_MAX_SCK_HZ (109*1000*1000)

#include "../generic/common.h"
