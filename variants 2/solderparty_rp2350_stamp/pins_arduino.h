#pragma once

#define PICO_RP2350A 1

// Pin definitions taken from:
//    https://rp2xxx-stamp-carrier-xl.solder.party/

// LEDs
#define PIN_LED        (3u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (24u)
#define PIN_SERIAL2_RX (25u)

// SPI
#define PIN_SPI0_MISO  (20u)
#define PIN_SPI0_MOSI  (23u)
#define PIN_SPI0_SCK   (22u)
#define PIN_SPI0_SS    (21u)

#define PIN_SPI1_MISO  (8u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (9u)

// SD Card connector
#define PIN_CARD_DETECT  (2u)
#define PIN_SD_CLK       (10u)
#define PIN_SD_CMD_MOSI  (11u)
#define PIN_SD_DAT0_MISO (8u)
#define PIN_SD_DAT3_CS   (9u)

// Wire
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)

#define PIN_WIRE1_SDA  (2u)
#define PIN_WIRE1_SCL  (3u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
