#pragma once

// Waveshare RP2350 Plus
// https://www.waveshare.com/wiki/RP2350-Plus
// https://files.waveshare.com/wiki/RP2350-Plus/RP2350_Plus.pdf
// https://www.waveshare.com/w/upload/3/3c/680px-RP2350-Plus-details-inter.jpg
//

/*
                  Pin#                Pin#
                       ___(_____)___
           GPIO0   1  |   *USB C*   | 40   VBUS
           GPIO1   2  |             | 39   VSYS
             GND   3  |             | 38   GND
           GPIO2   4  |             | 37   3V3_EN
           GPIO3   5  |             | 36   3V3(OUT)
           GPIO4   6  |             | 35   ADC_VREF
           GPIO5   7  |             | 34   GPIO28
             GND   8  |             | 33   GND
           GPIO6   9  |             | 32   GPIO27
           GPIO7  10  |             | 31   GPIO26
           GPIO8  11  |             | 30   RUN
           GPIO9  12  |             | 29   GPIO22
             GND  13  |             | 28   GND
          GPIO10  14  |             | 27   GPIO21
          GPIO11  15  |             | 25   GPIO20
          GPIO12  16  |             | 25   GPIO19
          GPIO13  17  |             | 24   GPIO18
             GND  18  |             | 23   GND
          GPIO14  19  |             | 22   GPIO17
          GPIO15  20  |____|_|_|____| 21   GPIO16
                           S G S
                           W N W
                           C D D
                           L   I
                           K   N
*/

#define PICO_RP2350A 1

// LEDs
#define PIN_LED (25u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
#define PIN_SPI0_MISO (16u)
#define PIN_SPI0_MOSI (19u)
#define PIN_SPI0_SCK (18u)
#define PIN_SPI0_SS (17u)

#define PIN_SPI1_MISO (12u)
#define PIN_SPI1_MOSI (15u)
#define PIN_SPI1_SCK (14u)
#define PIN_SPI1_SS (13u)

// Wire
#define PIN_WIRE0_SDA (8u)
#define PIN_WIRE0_SCL (9u)

#define PIN_WIRE1_SDA (6u)
#define PIN_WIRE1_SCL (7u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY (2u)
#define WIRE_HOWMANY (2u)

#include "../generic/common.h"
