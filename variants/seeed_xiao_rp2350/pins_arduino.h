#pragma once


// Pin definitions taken from:
//    https://www.seeedstudio.com/Seeed-XIAO-RP2350-p-5944.html




// LEDs
#define PIN_LED        (25u)


// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)


#define PIN_SERIAL2_TX (20u)
#define PIN_SERIAL2_RX (21u)


// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (5u)


#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (9u)


// Wire
#define PIN_WIRE0_SDA  (16u)
#define PIN_WIRE0_SCL  (17u)


#define PIN_WIRE1_SDA  (6u)
#define PIN_WIRE1_SCL  (7u)


#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)


#include "../generic/common.h"