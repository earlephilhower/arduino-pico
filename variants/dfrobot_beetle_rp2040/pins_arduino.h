#pragma once

// Pin definitions taken from:
//    https://www.dfrobot.com

// Not pinned out
#define PIN_LED        (13u)


// UART1
#define PIN_SERIAL1_TX (28u)
#define PIN_SERIAL1_RX (29u)

// UART2
#define PIN_SERIAL2_TX      (4u)
#define PIN_SERIAL2_RX      (5u)

// SPI
#define PIN_SPI0_MISO  (0u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (1u)

// Not pinned out
#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)

// Wire
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)
#define PIN_WIRE1_SDA  (2u)
#define PIN_WIRE1_SCL  (3u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
