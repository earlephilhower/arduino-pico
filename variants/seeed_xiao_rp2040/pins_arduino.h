#pragma once

// Pin definitions taken from:
//    https://www.seeedstudio.com/XIAO-RP2040-v1-0-p-5026.html

// LEDs
#define PIN_LED        (17u)
#define PIN_LED_R      (17u)
#define PIN_LED_G      (16u)
#define PIN_LED_B      (25u)

// NeoPixel
#define PIN_NEOPIXEL   (12u)
#define NEOPIXEL_POWER (11u)

// Serial1
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

// Serial2 not pinned out
#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (31u) // not pinned out

// Not pinned out
#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)

// Wire
#define PIN_WIRE0_SDA  (6u)
#define PIN_WIRE0_SCL  (7u)

// Wire1 not pinned out
#define PIN_WIRE1_SDA  (31u)
#define PIN_WIRE1_SCL  (31u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (1u)

#include "../generic/common.h"
