#pragma once

// Pin definitions taken from:
//    https://brtchip.com/ic-module/wp-content/uploads/sites/3/2022/07/DS_IDM2040-7A-Revised.pdf


// LEDs
#define PIN_LED        (25u)

// Serial
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
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (13u)

// default spi
#define PIN_SD_MOSI    PIN_SPI1_MOSI
#define PIN_SD_MISO    PIN_SPI1_MISO
#define PIN_SD_SCK     PIN_SPI1_SCK
#define PIN_SD_SS      PIN_SPI1_SS
#define SDCARD_DETECT  33

// Wire
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)

#define PIN_WIRE1_SDA  (26u)
#define PIN_WIRE1_SCL  (27u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
