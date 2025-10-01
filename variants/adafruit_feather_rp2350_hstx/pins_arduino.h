#pragma once

#define PICO_RP2350A 1

// LEDs
#define PIN_LED        (7u)

#define PIN_NEOPIXEL   (21)
#define NUM_NEOPIXEL   (1)

// UARTs
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)
#define PIN_SERIAL2_TX (31u) // not pinned out
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI0_MISO  (20u)
#define PIN_SPI0_MOSI  (23u)
#define PIN_SPI0_SCK   (22u)
#define PIN_SPI0_SS    (21u)
#define PIN_SPI1_MISO  (31u) // not pinned out
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)

// Wire
#define __WIRE0_DEVICE i2c1
#define PIN_WIRE0_SDA  (2u)
#define PIN_WIRE0_SCL  (3u)

#define __WIRE1_DEVICE i2c0
#define PIN_WIRE1_SDA  (31u) // not pinned out
#define PIN_WIRE1_SCL  (31u)

#define SERIAL_HOWMANY (1u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (1u)

// PSRAM
#define RP2350_PSRAM_CS         (8u)
#define RP2350_PSRAM_MAX_SCK_HZ (109*1000*1000)

// DVI connector
#define PIN_CKN (15u)
#define PIN_CKP (14u)
#define PIN_D0N (19u)
#define PIN_D0P (18u)
#define PIN_D1N (17u)
#define PIN_D1P (16u)
#define PIN_D2N (13u)
#define PIN_D2P (12u)

#include "../generic/common.h"
