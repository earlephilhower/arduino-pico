#pragma once

#include <stdint.h>

#include <cyw43_wrappers.h>

// Waveshare RP2350 Plus W
// https://www.waveshare.com/wiki/RP2350B-Plus-W
// https://files.waveshare.com/wiki/RP2350B-Plus-W/RP2350B-Plus-W.pdf
// https://www.waveshare.com/w/upload/4/41/RP2350B-Plus-W-details-inter.png
// NOTE: LED2: GPIO23
//
// NOTE: LED1: LED_WLGP0 B1_GPIO0 (Raspberry Pi Radio Module 2 / RMC20452T)
// RPI Pico 2 W also attaches an LED to WLGP0 on the CYW43439KUBG
// https://datasheets.raspberrypi.com/rm2/rm2-datasheet.pdf
// https://datasheets.raspberrypi.com/rm2/rm2-product-brief.pdf
// https://pip.raspberrypi.com/categories/1223-design-files
/*
                  Pin#                Pin#
                       ___(_____)___
           GPIO0   1  |   *USB C*   | 40   VBUS
           GPIO1   2  |             | 39   VSYS
             GND   3  |             | 38   GND
           GPIO2   4  |             | 37   3V3_EN
           GPIO3   5  |             | 36   3V3(OUT)
           GPIO4   6  |             | 35   ADC_VREF
           GPIO5   7  |             | 34   GPIO42
             GND   8  |             | 33   GND
           GPIO6   9  |             | 32   GPIO41
           GPIO7  10  |             | 31   GPIO40
           GPIO8  11  |             | 30   RUN
           GPIO9  12  |             | 29   GPIO22
             GND  13  |             | 28   GND
          GPIO10  14  |             | 27   GPIO21
          GPIO11  15  |             | 25   GPIO20
          GPIO12  16  |             | 25   GPIO19
          GPIO13  17  |             | 24   GPIO18
             GND  18  |  |_|_|_|_|  | 23   GND
          GPIO14  19  |  |_|_|_|_|  | 22   GPIO17
          GPIO15  20  |__|_|_|_|_|__| 21   GPIO16

                      43|33|30|27|24
                      --|--|--|--|--
                      44|34|31|28|25
                      --|--|--|--|--
                      45|35|32|29|26

    43  GPIO43  |  33  GPIO33  |  30  GPIO30  |  27  GPIO27  |  24  GPIO24
    44  GPIO44  |  34  GPIO34  |  31  GPIO31  |  28  GPIO28  |  25  GPIO25
    45  GPIO45  |  35  GPIO35  |  32  GPIO32  |  29  GPIO29  |  26  GPIO26

    "no debug"

    RP2350B MCU pinout  - NOTE: check schematic pinout
    --------------------------------------------------
    33 - SWCLK
    34 - SWDIO
*/

#define PICO_RP2350A 0 // RP2350B

// LEDs
#define LED_BUILTIN     (23u) // LED2 using the RP2350B
#define PIN_LED         (64u) // LED1 using the Raspberry Pi Radio Module 2
                              // "B1_GPIO0"

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
#define PIN_WIRE0_SDA   (8u)
#define PIN_WIRE0_SCL   (9u)

#define PIN_WIRE1_SDA   (6u)
#define PIN_WIRE1_SCL   (7u)

#define SERIAL_HOWMANY  (3u)
#define SPI_HOWMANY     (2u)
#define WIRE_HOWMANY    (2u)

#include "../generic/common.h"
