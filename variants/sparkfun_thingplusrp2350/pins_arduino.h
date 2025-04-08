#pragma once

#define PICO_RP2350A 1

#include <cyw43_wrappers.h>

// LEDs
#define PIN_LED        (64u)

#define PIN_NEOPIXEL   (14)
#define NUM_NEOPIXEL   (1)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (9u) // CS pin for SD card

#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)

// Wire
#define PIN_WIRE0_SDA  (6u)
#define PIN_WIRE0_SCL  (7u)

#define PIN_WIRE1_SDA  (31u)
#define PIN_WIRE1_SCL  (31u)

// Thing Plus uses I2C for Qwiic connector, make that the default
#ifndef __WIRE0_DEVICE
#define __WIRE0_DEVICE i2c1
#endif
#ifndef __WIRE1_DEVICE
#define __WIRE1_DEVICE i2c0
#endif

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (1u)

// PSRAM
#define RP2350_PSRAM_CS         (8u)
#define RP2350_PSRAM_MAX_SCK_HZ (109*1000*1000)

#include "../generic/common.h"
