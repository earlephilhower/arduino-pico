#pragma once

#define PICO_RP2350A 1

// DatanoiseTV PicoADK v2 - Audio Development Kit with RP2350A
// https://github.com/DatanoiseTV/PicoDSP-Hardware

// LEDs
#define PIN_LED        (2u)
#define LED_BUILTIN    PIN_LED

// Serial - relocated
#define PIN_SERIAL1_TX (12u)
#define PIN_SERIAL1_RX (13u)

// Serial 2 - relocated
#define PIN_SERIAL2_TX (27u)
#define PIN_SERIAL2_RX (28u)

// SPI0
#define PIN_SPI0_MISO  (8u)
#define PIN_SPI0_MOSI  (7u)
#define PIN_SPI0_SCK   (6u)
#define PIN_SPI0_SS    (5u)

// SPI1
#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (13u)

// Wire
#define PIN_WIRE0_SDA  (8u)
#define PIN_WIRE0_SCL  (9u)

#define PIN_WIRE1_SDA  (6u)
#define PIN_WIRE1_SCL  (7u)

// I2S
#define PIN_I2S_BCLK   (17u)
#define PIN_I2S_LRCLK  (18u)
#define PIN_I2S_DOUT   (16u)
#define PIN_I2S_DIN    (15u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

// PSRAM
#define RP2350_PSRAM_CS         (0u)
#define RP2350_PSRAM_MAX_SCK_HZ (109*1000*1000)
#define PIN_PSRAM_CS            RP2350_PSRAM_CS

// SDIO for SD Card
#define PIN_SDIO_CLK    (20u)
#define PIN_SDIO_CMD    (21u)
#define PIN_SDIO_D0     (22u)
#define PIN_SDIO_D1     (23u)
#define PIN_SDIO_D2     (24u)
#define PIN_SDIO_D3     (25u)

// MIDI
#define PIN_MIDI_RX    (1u)

#include "../generic/common.h"
