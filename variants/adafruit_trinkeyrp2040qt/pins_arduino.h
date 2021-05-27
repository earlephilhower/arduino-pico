#pragma once

// LEDs not pinned out
#define PIN_LED        (31u)

#define PIN_NEOPIXEL   (27u)
#define NUM_NEOPIXEL   (1u)
#define PIN_SWITCH     (12u)

// Serial, shared on QT pin
#define PIN_SERIAL1_TX (16u)
#define PIN_SERIAL1_RX (17u)

// Not pinned out
#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI Not pinned out
#define PIN_SPI0_MISO  (31u)
#define PIN_SPI0_MOSI  (31u)
#define PIN_SPI0_SCK   (31u)
#define PIN_SPI0_SS    (31u)

// Not pinned out
#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)

// Wire
#define PIN_WIRE0_SDA  (16u)
#define PIN_WIRE0_SCL  (17u)

// Wire1  Not pinned out
#define PIN_WIRE1_SDA  (31u)
#define PIN_WIRE1_SCL  (31u)

#define SERIAL_HOWMANY (1u)
#define SPI_HOWMANY    (0u)
#define WIRE_HOWMANY   (1u)

#include "../generic/common.h"
