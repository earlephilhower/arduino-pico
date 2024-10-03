#pragma once

// Pin definitions taken from:
// https://github.com/Architeuthis-Flux/Jumperless/tree/main/Hardware

// LEDs
#define PIN_LED        (25u)

// Serial
#define PIN_SERIAL1_TX (16u)
#define PIN_SERIAL1_RX (17u)

#define PIN_SERIAL2_TX (18u) //generally unused
#define PIN_SERIAL2_RX (19u)

// SPI
#define PIN_SPI0_MISO  (0u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (1u)

// Wire
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)

#define PIN_WIRE1_SDA  (18u) //generally unused
#define PIN_WIRE1_SCL  (19u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
