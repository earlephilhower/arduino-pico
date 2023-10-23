#pragma once

// Pin definitions taken from:
//    https://github.com/joshajohnson/sea-picro/blob/master/documentation/pinout/sea-picro-top-pinout-rory-render.png

// LEDs not pinned out
#define PIN_LED (31u)

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
#define PIN_SPI0_SS (31u) // not pinned out

// Not pinned out
#define PIN_SPI1_MISO (31u)
#define PIN_SPI1_MOSI (31u)
#define PIN_SPI1_SCK (31u)
#define PIN_SPI1_SS (31u)

// Wire is on the STEMMA QT port
#define PIN_WIRE0_SDA (2u)
#define PIN_WIRE0_SCL (3u)

// Not pinned out
#define PIN_WIRE1_SDA (31u)
#define PIN_WIRE1_SCL (31u)

#define SERIAL_HOWMANY (1u)
#define SPI_HOWMANY (1u)
#define WIRE_HOWMANY (1u)

#include "../generic/common.h"
