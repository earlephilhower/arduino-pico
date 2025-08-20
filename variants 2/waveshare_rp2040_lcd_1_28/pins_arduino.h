#pragma once

// Waveshare RP2040 LCD 1.28
// https://www.waveshare.com/wiki/RP2040-LCD-1.28
// https://www.waveshare.com/w/upload/6/60/RP2040-LCD-1.28-sch.pdf
// https://www.waveshare.com/img/devkit/RP2040-LCD-1.28/RP2040-LCD-1.28-details-3.jpg
//

/*
                 H1                                H2
            Pin#    Pin#                      Pin#    Pin#
    GPIO8   1       2    GPIO0        GND     1       2   GND
    GPIO9   3       4    GPIO1        VSYS    3       4   ADC_AVDD
    GPIO10  5       6    GPIO2        GPIO23  5       6   BOOT
    GPIO11  7       8    GPIO3        GPIO22  7       8   RUM
    GPIO12  9       10   GPIO4        GPIO21  9       10  GPIO29
    GPIO13  11      12   GPIO5        GPIO20  11      12  GPIO28
    GPIO14  13      14   GPIO6        GPIO19  13      14  GPIO27
    GPIO15  15      16   GPIO7        GPIO18  15      16  GPIO26
    SWCLK   17      18   VSYS         GPIO17  17      18  GPIO25
    SWDIP   19      20   GND          GPIO16  19      20  GPIO24
*/

// LCD
#define LDC_SPI         (1u)
#define PIN_LCD_DC      (8u)
#define PIN_LCD_CS      (9u)
#define PIN_LCD_SCLK    (10u)
#define PIN_LCD_MOSI    (11u)
#define PIN_LCD_RST     (12u)
#define PIN_LCD_BL      (25u)
// BAT_ADC
#define PIN_BAT_ADC      (29u)
// IMU
#define PIN_IMU_SDA      (6u)
#define PIN_IMU_SCL      (7u)
#define PIN_IMU_INT1     (23u)
#define PIN_IMU_INT2     (24u)

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

