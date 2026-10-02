#pragma once

// Waveshare RP2350-POE-ETH
// https://www.waveshare.com/rp2350-poe-eth.htm
// https://docs.waveshare.com/RP2350-POE-ETH
// https://files.waveshare.com/wiki/RP2350-POE-ETH/RP2350-POE-ETH_Logic.pdf

/*
    GPIO0  - GPIO15  General purpose I/O (header pins)
    GPIO16           SPI0 RX  (W6300 MISO)
    GPIO17           SPI0 CSn (W6300 CS)
    GPIO18           SPI0 SCK (W6300 SCK)
    GPIO19           SPI0 TX  (W6300 MOSI)
    GPIO20           W6300 RSTn
    GPIO21           W6300 INTn
    GPIO22           General purpose I/O
    GPIO23           PoE detect
    GPIO24           USB detect
    GPIO25           WS2812 data in
    GPIO26 - GPIO28  General purpose I/O
    GPIO29           ADC3
*/

#define PICO_RP2350A 1

// W6300 Ethernet
#define PIN_W6300_MISO  (16u)
#define PIN_W6300_CS    (17u)
#define PIN_W6300_SCK   (18u)
#define PIN_W6300_MOSI  (19u)
#define PIN_W6300_RST   (20u)
#define PIN_W6300_INT   (21u)

// PoE / USB power detection
#define PIN_POE_DET     (23u)
#define PIN_USB_DET     (24u)

// NeoPixel
#define PIN_NEOPIXEL    (25u)

// Serial
#define PIN_SERIAL1_TX  (0u)
#define PIN_SERIAL1_RX  (1u)

#define PIN_SERIAL2_TX  (8u)
#define PIN_SERIAL2_RX  (9u)

// SPI
#define PIN_SPI0_MISO   (16u)
#define PIN_SPI0_MOSI   (19u)
#define PIN_SPI0_SCK    (18u)
#define PIN_SPI0_SS     (17u)

#define PIN_SPI1_MISO   (12u)
#define PIN_SPI1_MOSI   (15u)
#define PIN_SPI1_SCK    (14u)
#define PIN_SPI1_SS     (13u)

// Wire
#define PIN_WIRE0_SDA   (4u)
#define PIN_WIRE0_SCL   (5u)

#define PIN_WIRE1_SDA   (26u)
#define PIN_WIRE1_SCL   (27u)

#define SERIAL_HOWMANY  (3u)
#define SPI_HOWMANY     (2u)
#define WIRE_HOWMANY    (2u)

#include "../generic/common.h"
