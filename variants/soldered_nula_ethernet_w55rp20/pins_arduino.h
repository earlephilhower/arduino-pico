#pragma once

//LEDs
#define PIN_NEOPIXEL   (1u)
#define NUM_NEOPIXEL   (1u)

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

// SD Card SPI
#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (13u)

// Wire
#define __WIRE0_DEVICE (i2c1)
#define PIN_WIRE0_SDA  (2u)
#define PIN_WIRE0_SCL  (3u)

#define __WIRE1_DEVICE (i2c0)
#define PIN_WIRE1_SDA  (28u)
#define PIN_WIRE1_SCL  (29u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

// SD Card detect - Active Low
static const uint8_t SD_ENABLE = (9u);


#include "../generic/common.h"
