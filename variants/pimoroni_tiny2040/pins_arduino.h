#pragma once

// This is a bare board with few predefined pins, so based on generic

// Pin definitions taken from:
//    https://datasheets.raspberrypi.org/pico/pico-datasheet.pdf
//    https://shop.pimoroni.com/products/tiny-2040

// RGB LED on pins 18-20.  Use the Green LED element on pin 19 as LED_BUILTIN
#define PIN_LED        (19u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (4u)
#define PIN_SERIAL2_RX (5u)

// SPI = SPI0 only, on GPI0 0-3 or 4-7 (no SS)
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (7u)
#define PIN_SPI0_SCK   (6u)
// PIN_SPI0_SS not available on any header pin but needs to be defined for ../generic/common.h so use a dummy
#define PIN_SPI0_SS    (17u)

// SPI1 not available - not enough pins

// Wire = WIRE0 on 0-1 or 4-5, WIRE1 on 2-3 or 6-7
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)

#define PIN_WIRE1_SDA  (6u)
#define PIN_WIRE1_SCL  (7u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
