#pragma once

// Pin definitions taken from:
//    https://learn.adafruit.com/assets/100337

// LEDs
#define PIN_LED        (11u)

// NeoPixel
#define PIN_NEOPIXEL   (17u)
#define NEOPIXEL_POWER (16u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

// Not pinned out
#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI0_MISO  (20u)
#define PIN_SPI0_MOSI  (19u)
#define PIN_SPI0_SCK   (18u)
#define PIN_SPI0_SS    (31u) // not pinned out

// Not pinned out
#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)

// Wire
#define PIN_WIRE0_SDA  (24u)
#define PIN_WIRE0_SCL  (25u)
#define PIN_WIRE1_SDA  (2u)
#define PIN_WIRE1_SCL  (3u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (2u)

// D pins
#define __PIN_D2             (12u)
#define __PIN_D3             (5u)
#define __PIN_D4             (4u)
#define __PIN_D5             (14u)
#define __PIN_D7             (6u)
#define __PIN_D9             (7u)
#define __PIN_D10            (8u)
#define __PIN_D11            (9u)
#define __PIN_D12            (10u)
#define __PIN_D13            (11u)

#include "../generic/common.h"
