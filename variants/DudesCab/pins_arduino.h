#pragma once

// Pin definitions taken from:
//    https://datasheets.raspberrypi.org/pico/pico-datasheet.pdf


// LEDs
#define PIN_LED        (25u)

// Serial
#define PIN_SERIAL1_TX (8u)
#define PIN_SERIAL1_RX (9u)
#define PIN_SERIAL2_TX (0u)
#define PIN_SERIAL2_RX (1u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (5u)

// Wire
#define PIN_WIRE0_SDA  (0u)
#define PIN_WIRE0_SCL  (1u)
#define PIN_WIRE0_SDA  (0u)
#define PIN_WIRE0_SCL  (1u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
