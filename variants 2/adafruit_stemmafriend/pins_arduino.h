#pragma once

// Pin definitions taken from:
//    https://learn.adafruit.com/assets/100337

// LEDs not pinned out
#define PIN_LED        (12u)  // backlight, weird but why not?

#define PIN_NEOPIXEL   (17u)
#define NUM_NEOPIXEL   (1u)
#define PIN_SWITCH     (11u)

// Serial
#define PIN_SERIAL1_TX (26u)  // shared on grove/jst 2mm
#define PIN_SERIAL1_RX (27u)  // shared on grove/jst 2mm

// Not pinned out
#define PIN_SERIAL2_TX (20u)  // shared on JST SH
#define PIN_SERIAL2_RX (21u)

// SPI
#define PIN_SPI0_MISO  (4u)  // unconnected
#define PIN_SPI0_MOSI  (7u)  // TFT data
#define PIN_SPI0_SCK   (2u)  // TFT clock
#define PIN_SPI0_SS    (1u)  // TFT CS

// Not pinned out
#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)

// Wire connected to STEMMA QT
#define PIN_WIRE0_SDA  (20u)
#define PIN_WIRE0_SCL  (21u)

// Wire1 is connected to Stemma JST/grove connector
#define PIN_WIRE1_SDA  (26u)
#define PIN_WIRE1_SCL  (27u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
