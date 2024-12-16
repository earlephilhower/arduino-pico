#pragma once

// Pin definitions taken from:
//    https://datasheets.raspberrypi.org/pico/pico-datasheet.pdf


// LEDs
#define PIN_LED        (25u)

// Serial
#define PIN_SERIAL1_TX (30u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (30u)
#define PIN_SERIAL2_RX (30u)

// SPI
#define PIN_SPI0_MISO  (12u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (13u)

#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (30u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (5u)

// Wire
#define PIN_WIRE0_SDA  (16u)
#define PIN_WIRE0_SCL  (17u)

#define PIN_WIRE1_SDA  (5u)
#define PIN_WIRE1_SCL  (4u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
