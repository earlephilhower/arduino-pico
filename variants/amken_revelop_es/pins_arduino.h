#pragma once


// LEDs
#define PIN_LED           (5u)

// NeoPixel
#define PIN_NEOPIXEL      (5u)
#define NEOPIXEL_POWER    (20u)


// CAN bus
#define PIN_CAN_CS        (9u)
#define PIN_CAN_INTERRUPT (29u)

//Accelerometer
#define PIN_LIS_CS         (1u)
#define PIN_LIS_INTERRUPT1 (23u)
#define PIN_LIS_INTERRUPT1 (25u)

//MAX31865
#define PIN_MAX31865_CS     (24u)

// Serial
#define PIN_SERIAL1_TX    (20u)
#define PIN_SERIAL1_RX    (31u)

// Not pinned out
#define PIN_SERIAL2_TX    (31u)
#define PIN_SERIAL2_RX    (31u)

// Shared between LIS2D and MAX31865
#define PIN_SPI0_MISO     (0u)
#define PIN_SPI0_MOSI     (3u)
#define PIN_SPI0_SCK      (2u)
#define PIN_SPI0_SS       (1u)
#define __SPI0_DEVICE     spi1

// CAN
#define PIN_SPI1_MISO     (11u)
#define PIN_SPI1_MOSI     (8u)
#define PIN_SPI1_SCK      (10u)
#define PIN_SPI1_SS       (31u)
#define __SPI1_DEVICE     spi0

// Wire
#define PIN_WIRE0_SDA     (31u)
#define PIN_WIRE0_SCL     (31u)
#define __WIRE0_DEVICE    i2c1

#define PIN_WIRE1_SDA     (31u)
#define PIN_WIRE1_SCL     (31u)
#define __WIRE1_DEVICE    i2c0

#define SERIAL_HOWMANY    (1u)
#define SPI_HOWMANY       (2u)
#define WIRE_HOWMANY      (0u)

#include "../generic/common.h"
