#pragma once

// Pin definitions taken from:
//    https://datasheets.raspberrypi.org/pico/pico-datasheet.pdf


// LEDs
#define PIN_LED        (19u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
// Default SPI0 pins are used internally and are unavailable on this board. To use SPI0 other peripherals must be sacrificed.
#define PIN_SPI0_MISO  (2u) // 16u is connected to the buck-boost converted.
#define PIN_SPI0_MOSI  (3u) // 19u is connected to USERLED, defined as PIN_LED.
#define PIN_SPI0_SCK   (4u) // 18u is connected to a voltage divider on VBUS.
#define PIN_SPI0_SS    (5u) // 17u doesn't even show up in the schematics.

#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (15u)
#define PIN_SPI1_SCK   (14u)
#define PIN_SPI1_SS    (13u)

// Wire
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)

#define PIN_WIRE1_SDA  (26u)
#define PIN_WIRE1_SCL  (27u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
