#pragma once

// Waveshare RP2350 Core
// https://www.waveshare.com/wiki/Core2350B0
// https://files.waveshare.com/wiki/Core2350B0/Core2350B.pdf

#define PICO_RP2350A 0 // RP2350B
#define RP2350_PSRAM_CS (47u)

// LED
#define PIN_LED (39u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (4u)
#define PIN_SERIAL2_RX (5u)

// SPI
#define PIN_SPI0_MISO (16u)
#define PIN_SPI0_MOSI (19u)
#define PIN_SPI0_SCK (18u)
#define PIN_SPI0_SS (17u)

#define PIN_SPI1_MISO (24u)
#define PIN_SPI1_MOSI (27u)
#define PIN_SPI1_SCK (26u)
#define PIN_SPI1_SS (25u)

// Wire
#define PIN_WIRE0_SDA (8u)
#define PIN_WIRE0_SCL (9u)

#define PIN_WIRE1_SDA (10u)
#define PIN_WIRE1_SCL (11u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY (2u)
#define WIRE_HOWMANY (2u)

#include "../generic/common.h"
