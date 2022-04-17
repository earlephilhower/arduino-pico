#pragma once

// Taken from schematic at https://cdn.sparkfun.com/assets/e/2/7/6/b/ProMicroRP2040_Graphical_Datasheet.pdf

// LEDs
#define PIN_LED        (25u)

#define PIN_NEOPIXEL   (8u)
#define NUM_NEOPIXEL   (1)

// UARTs
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (31u)

#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (15u)
#define PIN_SPI1_SCK   (14u)
#define PIN_SPI1_SS    (9u)

// Wire
#define PIN_WIRE0_SDA  (16u)
#define PIN_WIRE0_SCL  (17u)

#define PIN_WIRE1_SDA  (6u)
#define PIN_WIRE1_SCL  (7u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (1u)

#include "../generic/common.h"
