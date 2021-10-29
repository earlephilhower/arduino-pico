#pragma once

// LEDs
#define PIN_LED             (3u)

// Neopixel
#define PIN_NEOPIXEL        (18u)
#define NUM_NEOPIXEL        (2u)
#define PIN_RGB             PIN_NEOPIXEL

// Buzzer
#define PIN_BUZZER          (22u)

// Motor Driver
#define PIN_M1A	            (8u)
#define PIN_M1B	            (9u)
#define PIN_M2A	            (10u)
#define PIN_M2B	            (11u)

// Voltage Monitor
#define PIN_VBATT           (29u)
#define PIN_VOLTAGE_MONITOR PIN_VBATT



// Serial
#define PIN_SERIAL1_TX      (0u)
#define PIN_SERIAL1_RX      (1u)

#define PIN_SERIAL2_TX      (4u)
#define PIN_SERIAL2_RX      (5u)

// SPI (Not pinned out)
#define PIN_SPI0_MISO       (31u)
#define PIN_SPI0_MOSI       (31u)
#define PIN_SPI0_SCK        (31u)
#define PIN_SPI0_SS         (31u)

#define PIN_SPI1_MISO       (31u)
#define PIN_SPI1_MOSI       (31u)
#define PIN_SPI1_SCK        (31u)
#define PIN_SPI1_SS         (31u)

// Wire
#define PIN_WIRE0_SDA       (16u)
#define PIN_WIRE0_SCL       (17u)

#define PIN_WIRE1_SDA       (2u)
#define PIN_WIRE1_SCL       (3u)



#define SERIAL_HOWMANY      (2u)
#define SPI_HOWMANY         (0u)
#define WIRE_HOWMANY        (2u)

#include "../generic/common.h"
