#pragma once

// METE HOCA Akana R1 Pin Definitions

// LEDs
#define PIN_LED        (25u)
#define PIN_NEO        (24u)

// Buttons
#define BTN_ENTER      (14u)
#define BTN_BACK       (15u)
#define BTN_UP         (22u)
#define BTN_DOWN       (23u)
#define BTN_LEFT       (20u)
#define BTN_RIGHT      (21u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
#define PIN_SPI0_MISO  (16u) // RX
#define PIN_SPI0_MOSI  (19u) // TX
#define PIN_SPI0_SCK   (18u) // SCK
#define PIN_SPI0_SS    (17u) // CSn

#define PIN_SPI1_MISO  (12u) // RX
#define PIN_SPI1_MOSI  (11u) // TX
#define PIN_SPI1_SCK   (10u) // SCK
#define PIN_SPI1_SS    (13u) // CSn

// Wire
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)

#define PIN_WIRE1_SDA  (6u)
#define PIN_WIRE1_SCL  (7u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
