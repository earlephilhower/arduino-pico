#pragma once

// Pin definitions taken from:
//    https://learn.adafruit.com/assets/100337

// LEDs not pinned out
#define PIN_LED        (31u)

// NeoPixel
#define PIN_NEOPIXEL   (12u)
#define NEOPIXEL_POWER (11u)

// Serial1
#define PIN_SERIAL1_TX (28u) // marked A1 on the Board
#define PIN_SERIAL1_RX (29u) // marked A0 on the Board

// Serial2
#define PIN_SERIAL2_TX (20u) // marked TX on the Board
#define PIN_SERIAL2_RX (5u)  // marked RX on the Board

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
