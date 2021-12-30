#pragma once

// Pin definitions taken from:
//    https://learn.adafruit.com/assets/100337

// LEDs not pinned out
#define PIN_LED        (31u)

// Serial
#define PIN_SERIAL1_TX (20u)
#define PIN_SERIAL1_RX (5u)

// Not pinned out
#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (6u)
#define PIN_SPI0_SS    (31u) // not pinned out

// Not pinned out
#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)

// Wire
#define PIN_WIRE0_SDA  (24u)
#define PIN_WIRE0_SCL  (25u)

// Wire1 is connected to StemmaQT connector
#define PIN_WIRE1_SDA  (22u)
#define PIN_WIRE1_SCL  (23u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (2u)

// Pin overrides specific to the QT Py RP2040
#define __PIN_A0 (29u)
#define __PIN_A1 (28u)
#define __PIN_A2 (27u)
#define __PIN_A3 (26u)

#include "../generic/common.h"
