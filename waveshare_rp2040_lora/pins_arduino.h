#pragma once

// Waveshare RP2040 LoRa
// https://www.waveshare.com/wiki/RP2040-LoRa
// https://files.waveshare.com/wiki/RP2040-LoRa/Rp2040-lora-sch.pdf
// https://www.waveshare.com/w/upload/8/82/RP2040-LoRa_Pinout.jpg
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
          GPIO11  15  |             | 26   GPIO20
          GPIO12  16  |_____________| 25   GPIO19
*/

// LEDs
#define PIN_LED        (25u)

// LoRa module internal connections
#define SX1262_CS      (13u)
#define SX1262_CLK     (14u)
#define SX1262_MOSI    (15u)
#define SX1262_DIO1    (16u)
#define SX1262_ANT_SW  (17u)
#define SX1262_BUSY    (18u)
#define SX1262_RST     (23u)
#define SX1262_MISO    (24u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (5u)

#define PIN_SPI1_MISO  (8u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (9u)

// Wire
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)

#define PIN_WIRE1_SDA  (6u)
#define PIN_WIRE1_SCL  (7u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)


#include "../generic/common.h"
