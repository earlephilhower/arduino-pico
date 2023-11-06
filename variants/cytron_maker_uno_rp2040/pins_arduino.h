#pragma once

// LEDs
#define PIN_LED             (3u)


// Neopixel
#define PIN_NEOPIXEL        (25u)
#define NUM_NEOPIXEL        (2u)
#define PIN_RGB             PIN_NEOPIXEL

// Buzzer
#define PIN_BUZZER          (8u)

// Button
#define PIN_BUTTON	    (2u)



// Serial 1
#define PIN_SERIAL1_TX      (0u)
#define PIN_SERIAL1_RX      (1u)

// Serial 2 (Not pinned out)
#define PIN_SERIAL2_TX      (31u)
#define PIN_SERIAL2_RX      (31u)

// SPI 0
#define PIN_SPI0_MISO       (12u)
#define PIN_SPI0_MOSI       (11u)
#define PIN_SPI0_SCK        (10u)
#define PIN_SPI0_SS         (13u)

// SPI 1 (Not pinned out)
#define PIN_SPI1_MISO       (31u)
#define PIN_SPI1_MOSI       (31u)
#define PIN_SPI1_SCK        (31u)
#define PIN_SPI1_SS         (31u)

// Wire
#define PIN_WIRE0_SDA       (20u)
#define PIN_WIRE0_SCL       (21u)

#define PIN_WIRE1_SDA       (26u)
#define PIN_WIRE1_SCL       (27u)



#define SERIAL_HOWMANY      (1u)
#define SPI_HOWMANY         (1u)
#define WIRE_HOWMANY        (1u)

#include "../generic/common.h"
