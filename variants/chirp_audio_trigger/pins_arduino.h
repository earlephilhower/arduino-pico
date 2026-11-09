#pragma once

#define PICO_RP2350A 1

// Pin definitions for the CHIRP Audio Trigger
// https://github.com/joymonkey/CHIRP

// LEDs (3 NeoPixels on GPIO 19)
#define PIN_NEOPIXEL   (19u)
#define NUM_NEOPIXEL   (3)

// Serial / UARTs
// Hardware UART (Serial2 on GPIO 4 TX / GPIO 5 RX)
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)
#define PIN_SERIAL2_TX (4u)
#define PIN_SERIAL2_RX (5u)

// SPI Interfaces
// SPI1: Dedicated MicroSD Card Socket
#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (15u)
#define PIN_SPI1_SCK   (14u)
#define PIN_SPI1_SS    (13u)

#define PIN_SPI0_MISO  (31u)
#define PIN_SPI0_MOSI  (31u)
#define PIN_SPI0_SCK   (31u)
#define PIN_SPI0_SS    (31u)

// Wire / I2C (not pinned out by default)
#define PIN_WIRE0_SDA  (31u)
#define PIN_WIRE0_SCL  (31u)
#define PIN_WIRE1_SDA  (31u)
#define PIN_WIRE1_SCL  (31u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (0u)

// I2S DAC (PCM5102A)
#define PIN_I2S_DATA   (11u)
#define PIN_I2S_BCLK   (9u)
#define PIN_I2S_LRCK   (10u)
#define PIN_I2S_MUTE   (8u)

// Navigation Buttons & Controls
#define PIN_BTN_FWD    (16u) // Next
#define PIN_BTN_NAV    (17u) // Play/Stop
#define PIN_BTN_REV    (18u) // Prev
#define PIN_MSC_TRIGGER (7u) // Pull low for USB Mass Storage mode

// PSRAM (8MB with CS on GPIO 0)
#define RP2350_PSRAM_CS         (0u)
#define RP2350_PSRAM_MAX_SCK_HZ (133*1000*1000)

#include "../generic/common.h"
