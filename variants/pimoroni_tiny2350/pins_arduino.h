#pragma once

#define PICO_RP2350A 1

// This is a bare board with few predefined pins, so based on generic

// Pin definitions taken from:
//    https://datasheets.raspberrypi.org/pico/pico-datasheet.pdf
//    https://cdn.shopify.com/s/files/1/0174/1800/files/tiny2350_pinout_diagram.png

// RGB LED on pins 18-20.  Use the Green LED element on pin 19 as LED_BUILTIN
#define PIN_LED        (19u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (4u)
#define PIN_SERIAL2_RX (5u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (7u)
#define PIN_SPI0_SCK   (6u)
#define PIN_SPI0_SS    (5u)

#define PIN_SPI1_MISO  (28u)
#define PIN_SPI1_MOSI  (27u)
#define PIN_SPI1_SCK   (26u)
#define PIN_SPI1_SS    (29u)

// Wire = WIRE0 on 0-1 or 4-5, WIRE1 on 2-3 or 6-7
#define PIN_WIRE0_SDA  (12u)
#define PIN_WIRE0_SCL  (13u)

#define PIN_WIRE1_SDA  (6u)
#define PIN_WIRE1_SCL  (7u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
