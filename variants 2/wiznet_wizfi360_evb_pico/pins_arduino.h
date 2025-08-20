#pragma once

// LEDs
#define PIN_LED        (25u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

// Slight difference to generic RP2040
// PIN_SERIAL2_TX + PIN_SERIAL2_RX are
// switched with PIN_WIRE0_SDA + PIN_WIRE0_SCL
#define PIN_SERIAL2_TX (4u)
#define PIN_SERIAL2_RX (5u)

// SPI
#define PIN_SPI0_MISO  (16u)
#define PIN_SPI0_MOSI  (19u)
#define PIN_SPI0_SCK   (18u)
#define PIN_SPI0_SS    (17u)

#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (15u)
#define PIN_SPI1_SCK   (14u)
#define PIN_SPI1_SS    (13u)

// Wire
// Slight difference to generic RP2040
// PIN_SERIAL2_TX + PIN_SERIAL2_RX are
// switched with PIN_WIRE0_SDA + PIN_WIRE0_SCL
#define PIN_WIRE0_SDA  (8u)
#define PIN_WIRE0_SCL  (9u)

#define PIN_WIRE1_SDA  (26u)
#define PIN_WIRE1_SCL  (27u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
