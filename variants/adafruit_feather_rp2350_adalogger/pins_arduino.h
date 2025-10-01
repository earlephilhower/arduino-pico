#pragma once

#define PICO_RP2350A 1

// LEDs
#define PIN_LED        (7u)

#define PIN_NEOPIXEL   (21u)
#define NUM_NEOPIXEL   (1)

// SD Card connector
#define PIN_CARD_DETECT (13u)
#define PIN_SD_CLK (14u)
#define PIN_SD_CMD_MOSI (15u)
#define PIN_SD_DAT0_MISO (16u)
#define PIN_SD_DAT1 (17u)
#define PIN_SD_DAT2 (18u)
#define PIN_SD_DAT3_CS (19u)

// UARTs
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)
#define PIN_SERIAL2_TX (99u) // not pinned out
#define PIN_SERIAL2_RX (99u)

// SPI
#define PIN_SPI0_MISO  (20u)
#define PIN_SPI0_MOSI  (23u)
#define PIN_SPI0_SCK   (22u)
#define PIN_SPI0_SS    (13u)
#define __SPI0_DEVICE  spi0

// SPI1 for SD card
#define PIN_SPI1_MISO  PIN_SD_DAT0_MISO
#define PIN_SPI1_MOSI  PIN_SD_CMD_MOSI
#define PIN_SPI1_SCK   PIN_SD_CLK
#define PIN_SPI1_SS    PIN_SD_DAT3_CS
#define __SPI1_DEVICE  spi1

// Wire
#define __WIRE0_DEVICE i2c0
#define PIN_WIRE0_SDA  (2u)
#define PIN_WIRE0_SCL  (3u)

#define __WIRE1_DEVICE i2c1
#define PIN_WIRE1_SDA  (31u) // not pinned out
#define PIN_WIRE1_SCL  (31u)

#define SERIAL_HOWMANY (1u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (1u)

#include "../generic/common.h"
