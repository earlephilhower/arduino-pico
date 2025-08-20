#pragma once

// DatanoiseTV PicoADK+ - Audio Development Kit
// Pin definitions taken from:
// https://github.com/DatanoiseTV/PicoDSP-Hardware

// LEDs
#define PIN_LED        (15u)

// Debug LEDs near the USB connector
#define PIN_LED0       (2u)
#define PIN_LED1       (3u)
#define PIN_LED2       (4u)
#define PIN_LED3       (5u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI0
#define PIN_SPI0_MISO  (16u)
#define PIN_SPI0_MOSI  (19u)
#define PIN_SPI0_SCK   (18u)
#define PIN_SPI0_SS    (17u)

// SPI1
#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (13u)

// Wire
#define PIN_WIRE0_SDA  (8u)
#define PIN_WIRE0_SCL  (9u)

#define PIN_WIRE1_SDA  (6u)
#define PIN_WIRE1_SCL  (7u)

// I2S
#define PIN_I2S_BCLK   (17u)
#define PIN_I2S_LRCLK  (18u)
#define PIN_I2S_DOUT   (16u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
