#pragma once

// Pin definitions taken from:
//    https://archikids.com.ar/documentacion

//Buttons
#define PIN_BUTTON_A   (23u)
#define PIN_BUTTON_B   (10u)
#define PIN_BUTTON_C   (25u)
#define PIN_BUTTON_D   (19u)

// NeoPixel
#define PIN_NEOPIXEL   (24u)
#define NUM_NEOPIXEL    64
#define PIN_RGB         PIN_NEOPIXEL

// Buzzer
#define PIN_BUZZER     (22u)

// Serial
#define PIN_SERIAL1_TX (16u)
#define PIN_SERIAL1_RX (17u)

#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (5u)

#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (13u)

// Wire
#define PIN_WIRE0_SDA  (0u)
#define PIN_WIRE0_SCL  (1u)

#define PIN_WIRE1_SDA  (6u)
#define PIN_WIRE1_SCL  (7u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
