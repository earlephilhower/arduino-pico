#pragma once

// Pin definitions taken from:
//    https://learn.adafruit.com/assets/100337

// LEDs
#define PIN_LED        (13u)

// NeoPixel
#define PIN_NEOPIXEL   (4u)

// 'Boot0' button also on GPIO #7
#define PIN_BUTTON     (7u)

// SCORPIO end connector (level-shifted I/O)
#define PIN_NEOPIXEL0 (16u)
#define PIN_NEOPIXEL1 (17u)
#define PIN_NEOPIXEL2 (18u)
#define PIN_NEOPIXEL3 (19u)
#define PIN_NEOPIXEL4 (20u)
#define PIN_NEOPIXEL5 (21u)
#define PIN_NEOPIXEL6 (22u)
#define PIN_NEOPIXEL7 (23u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

// Not pinned out
#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI0_MISO  (8u)
#define PIN_SPI0_MOSI  (15u)
#define PIN_SPI0_SCK   (14u)
#define PIN_SPI0_SS    (13u)
#define __SPI0_DEVICE  spi1

// Not pinned out
#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)
#define __SPI1_DEVICE  spi0

// Wire
#define PIN_WIRE0_SDA  (2u)
#define PIN_WIRE0_SCL  (3u)
#define __WIRE0_DEVICE i2c1

#define PIN_WIRE1_SDA  (31u)
#define PIN_WIRE1_SCL  (31u)
#define __WIRE1_DEVICE i2c0

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (1u)

#include "../generic/common.h"
