#pragma once

// LEDs
#define PIN_LED        (13u)

// NeoPixel
#define PIN_NEOPIXEL   (14u)

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
#define PIN_SPI0_SS    (23u)

// SDIO
#define PIN_SD_CLK       (18u)
#define PIN_SD_CMD_MOSI  (19u)
#define PIN_SD_DAT0_MISO (20u)
#define PIN_SD_DAT1      (21u)
#define PIN_SD_DAT2      (22u)
#define PIN_SD_DAT3_CS   (23u)
#define PIN_SD_DETECT    (15u)

// Not pinned out
#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)

// Wire
#define __WIRE0_DEVICE i2c0
#define PIN_WIRE0_SDA (16u)
#define PIN_WIRE0_SCL (17u)
#define __WIRE1_DEVICE i2c1
#define PIN_WIRE1_SDA (2u)
#define PIN_WIRE1_SCL (3u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (1u)

#include "../generic/common.h"
