#pragma once

// Waveshare RP2040 Matrix
// https://files.waveshare.com/upload/4/49/RP2040-Matrix.pdf
// Pins taken from https://github.com/earlephilhower/arduino-pico/files/14470983/WaveshareRP2040Matrix.hs.txt

// NeoPixel
#define PIN_NEOPIXEL   (16u)
#define NUM_NEOPIXEL   (25u)
//#define LED_BUILTIN PIN_NEOPIXEL

// Serial1
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (5u)

#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (15u)
#define PIN_SPI1_SCK   (14u)
#define PIN_SPI1_SS    (13u)

// Wire
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)

#define PIN_WIRE1_SDA  (26u)
#define PIN_WIRE1_SCL  (27u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
