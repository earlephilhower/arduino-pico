#pragma once

// LEDs
#define PIN_LED             (2u)

// Neopixel
#define PIN_NEOPIXEL        (11u)
#define NUM_NEOPIXEL        (2u)
#define PIN_RGB             PIN_NEOPIXEL

// Buzzer
#define PIN_BUZZER          (22u)

// Button
#define PIN_BUTTON			(20u)



// Serial 1
#define PIN_SERIAL1_TX      (0u)
#define PIN_SERIAL1_RX      (1u)

// Serial 2 (Not pinned out)
#define PIN_SERIAL2_TX      (31u)
#define PIN_SERIAL2_RX      (31u)

// SPI 0
#define PIN_SPI0_MISO       (16u)
#define PIN_SPI0_MOSI       (19u)
#define PIN_SPI0_SCK        (18u)
#define PIN_SPI0_SS         (17u)

// SPI 1 (Not pinned out)
#define PIN_SPI1_MISO       (31u)
#define PIN_SPI1_MOSI       (31u)
#define PIN_SPI1_SCK        (31u)
#define PIN_SPI1_SS         (31u)

// Wire
#define PIN_WIRE0_SDA       (0u)
#define PIN_WIRE0_SCL       (1u)

#define PIN_WIRE1_SDA       (26u)
#define PIN_WIRE1_SCL       (27u)



#define SERIAL_HOWMANY      (1u)
#define SPI_HOWMANY         (1u)
#define WIRE_HOWMANY        (2u)

#include "../generic/common.h"
