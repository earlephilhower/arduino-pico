#pragma once

// Pin definitions taken from: https://raw.githubusercontent.com/0xCB-dev/0xCB-Helios/main/rev1.0/helios.webp

// LEDs
#define PIN_LED (17u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

// Not pinned out
#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI0_MISO (20u)
#define PIN_SPI0_MOSI (23u)
#define PIN_SPI0_SCK (22u)
#define PIN_SPI0_SS (21u)

// Not pinned out
#define PIN_SPI1_MISO (31u)
#define PIN_SPI1_MOSI (31u)
#define PIN_SPI1_SCK (31u)
#define PIN_SPI1_SS (31u)

// Not pinned out
#define PIN_WIRE0_SDA (31u)
#define PIN_WIRE0_SCL (31u)

// Wire
#define PIN_WIRE1_SDA (2u)
#define PIN_WIRE1_SCL (3u)

#define SERIAL_HOWMANY (1u)
#define SPI_HOWMANY (1u)
#define WIRE_HOWMANY (1u)

#define PIN_NEOPIXEL (25u)
#include "../generic/common.h"
