#pragma once

#define PICO_RP2350A 0 // RP2350B
#define RP2350_PSRAM_CS (8u)

// LEDs
#define PIN_LED        (25u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (7u)
#define PIN_SPI0_SCK   (6u)
#define PIN_SPI0_SS    (5u)

#define PIN_SPI1_MISO  (24u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (9u)

// Wire
#define PIN_WIRE0_SDA  (12u)
#define PIN_WIRE0_SCL  (13u)

#define PIN_WIRE1_SDA  (2u)
#define PIN_WIRE1_SCL  (3u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

// DVI connector
#define PIN_CKN (15u)
#define PIN_CKP (14u)
#define PIN_D0N (13u)
#define PIN_D0P (12u)
#define PIN_D1N (19u)
#define PIN_D1P (18u)
#define PIN_D2N (17u)
#define PIN_D2P (16u)

#include "../generic/common.h"
