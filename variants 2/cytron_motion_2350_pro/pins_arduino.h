#pragma once

#define PICO_RP2350A 1

// LEDs
#define PIN_LED             (2u)

// Neopixel
#define PIN_NEOPIXEL        (23u)
#define NUM_NEOPIXEL        (2u)
#define PIN_RGB             PIN_NEOPIXEL

// USB host connector
#define PIN_USB_HOST_DP     (24u)
#define PIN_USB_HOST_DM     (25u)

// Buzzer
#define PIN_BUZZER          (22u)

// Motor Driver
#define PIN_M1A	            (8u)
#define PIN_M1B	            (9u)
#define PIN_M2A	            (10u)
#define PIN_M2B	            (11u)
#define PIN_M3A	            (12u)
#define PIN_M3B	            (13u)
#define PIN_M4A	            (14u)
#define PIN_M4B	            (15u)

// Voltage Monitor
#define PIN_VOLTAGE_MONITOR (29u)

// Serial
#define PIN_SERIAL1_TX      (0u)
#define PIN_SERIAL1_RX      (1u)

// Not pinned out
#define PIN_SERIAL2_TX      (31u)
#define PIN_SERIAL2_RX      (31u)

// SPI
#define PIN_SPI0_MISO       (4u)
#define PIN_SPI0_MOSI       (7u)
#define PIN_SPI0_SCK        (6u)
#define PIN_SPI0_SS         (5u)

// Not pinned out
#define PIN_SPI1_MISO       (31u)
#define PIN_SPI1_MOSI       (31u)
#define PIN_SPI1_SCK        (31u)
#define PIN_SPI1_SS         (31u)

// Wire
#define PIN_WIRE0_SDA       (16u)
#define PIN_WIRE0_SCL       (17u)

#define PIN_WIRE1_SDA       (26u)
#define PIN_WIRE1_SCL       (27u)

#define SERIAL_HOWMANY      (1u)
#define SPI_HOWMANY         (1u)
#define WIRE_HOWMANY        (2u)

#include "../generic/common.h"
