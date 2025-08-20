#pragma once

// LEDs
#define PIN_LED        (25u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

// ATTENTION -> USED by SPI0
#define PIN_SERIAL2_TX (4u)
#define PIN_SERIAL2_RX (5u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (5u)

// ATTENTION -> USED by ROM_SELECT Jumpers
#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (13u)

// ATTENTION -> USED by SPI0
// Wire
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (1u)

#include "../generic/common.h"

// GPIO  0 UART_TX_PIN 0
// GPIO  1 UART_RX_PIN 1
// GPIO  2 SPI SCK
// GOIO  3 SPI MOSI
// GPIO  4 SPI MISO
// GPIO  5 SPI CS/SS

// GPIO  6 Free(?)

// GPIO  7 RESET_BUTTON

// GPIO  8 Free(?)

// GPIO  9 DUMP_BUTTON
// GPIO 10 ROM_A13
// GPIO 11 ROM_A14
// GPIO 12 ROM_A15
// GPIO 13 SELSEL
// GPIO 14 SOUNDIO_1
// GPIO 15 SOUNDIO_2
// GPIO 22 HAS_SWITCHES_IO (former SD Card Detect)
