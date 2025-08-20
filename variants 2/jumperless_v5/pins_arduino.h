#pragma once

// Pin definitions taken from:
//    https://github.com/Architeuthis-Flux/JumperlessV5/tree/main/Hardware/Jumperless23V50

#define PICO_RP2350A   0 // RP2350B

// LEDs
#define PIN_LED        (17u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (24u)
#define PIN_SERIAL2_RX (25u)

// SPI
#define PIN_SPI0_MISO  (20u)
#define PIN_SPI0_MOSI  (23u)
#define PIN_SPI0_SCK   (22u)
#define PIN_SPI0_SS    (21u)

#define PIN_SPI1_MISO  (24u)
#define PIN_SPI1_MOSI  (27u)
#define PIN_SPI1_SCK   (26u)
#define PIN_SPI1_SS    (25u)

// Wire
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)

#define PIN_WIRE1_SDA  (22u)
#define PIN_WIRE1_SCL  (23u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
