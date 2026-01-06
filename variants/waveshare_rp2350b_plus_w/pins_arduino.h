#pragma once

#include <stdint.h>

#include <cyw43_wrappers.h>

// Waveshare RP2350 Plus W
// Pin definitionsL:
// https://www.waveshare.com/wiki/RP2350B-Plus-W
// https://files.waveshare.com/wiki/RP2350B-Plus-W/RP2350B-Plus-W.pdf
// https://www.waveshare.com/w/upload/4/41/RP2350B-Plus-W-details-inter.png

// NOTE: LED2: GPIO23
// NOTE: LED1: LED_WLGP0 B1_GPIO0 (Raspberry Pi Radio Module 2 / RMC20452T)

// Raspberry Pi Radio Module 2 / RMC20452T
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

    "no debug" is written on the schematic for the main connectors,
    but SWCLK and SWDIO are probably still accessible on these pads:
    33 - SWCLK
    34 - SWDIO
*/

#define PICO_RP2350A 0 // RP2350B

#define PINS_COUNT          (46u)
#define NUM_DIGITAL_PINS    (46u)
#define NUM_ANALOG_INPUTS   (7u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

// LED_BUILTIN
#define LED_BUILTIN     PIN_LED
#define PIN_LED         (64u) // LED1 uses the Raspberry Pi Radio Module 2 ("B1_GPIO0")

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

// PSRAM
// NOTE: does not come with a PSRAM chip
#define RP2350_PSRAM_CS         (47u)
// #define RP2350_PSRAM_MAX_SCK_HZ (109*1000*1000)

// DVI
#define PIN_CKN (15u)
#define PIN_CKP (14u)
#define PIN_D0N (13u)
#define PIN_D0P (12u)
#define PIN_D1N (19u)
#define PIN_D1P (18u)
#define PIN_D2N (17u)
#define PIN_D2P (16u)

// SPI
static const uint8_t SS = PIN_SPI0_SS;
static const uint8_t MOSI = PIN_SPI0_MOSI;
static const uint8_t MISO = PIN_SPI0_MISO;
static const uint8_t SCK = PIN_SPI0_SCK;

// Wire
static const uint8_t SDA = PIN_WIRE0_SDA;
static const uint8_t SCL = PIN_WIRE0_SCL;

static const uint8_t RX = PIN_SERIAL1_RX;
static const uint8_t TX = PIN_SERIAL1_TX;

/* Pins mappings for marked pins on the board */
// Digital
static const uint8_t D0 = (0u);
static const uint8_t D1 = (1u);
static const uint8_t D2 = (2u);
static const uint8_t D3 = (3u);
static const uint8_t D4 = (4u);
static const uint8_t D5 = (5u);
static const uint8_t D6 = (6u);
static const uint8_t D7 = (7u);
static const uint8_t D8 = (8u);
static const uint8_t D9 = (9u);
static const uint8_t D10 = (10u);
static const uint8_t D11 = (11u);
static const uint8_t D12 = (12u);
static const uint8_t D13 = (13u);
static const uint8_t D14 = (14u);
static const uint8_t D15 = (15u);
static const uint8_t D16 = (16u);
static const uint8_t D17 = (17u);
static const uint8_t D18 = (18u);
static const uint8_t D19 = (19u);
static const uint8_t D20 = (20u);
static const uint8_t D21 = (21u);
static const uint8_t D22 = (22u);
// These pins are still on the headers;  will probably be used more than the
// ones broken out onto the bare pads.
static const uint8_t D23 = (40u);
static const uint8_t D24 = (41u);
static const uint8_t D25 = (42u);

/* Pin mappings for the pads;  NOTE: P = "pad" */
static const uint8_t D26 = (24u); // GPIO24 = P1
static const uint8_t D27 = (25u); // GPIO25 = P3
static const uint8_t D28 = (26u); // GPIO26 = P5
static const uint8_t D29 = (27u); // GPIO27 = P7
static const uint8_t D30 = (28u); // GPIO28 = P9
static const uint8_t D31 = (29u); // GPIO29 = P11
static const uint8_t D32 = (30u); // GPIO30 = P13
static const uint8_t D33 = (31u); // GPIO31 = P15
static const uint8_t D34 = (32u); // GPIO32 = P2
static const uint8_t D35 = (33u); // GPIO33 = P4
static const uint8_t D36 = (34u); // GPIO34 = P6
static const uint8_t D37 = (35u); // GPIO35 = P8

/* Pins connected to the Raspberry Pi Radio Module 2 */
// GPIO36 = WL_ON
// GPIO37 = WL_D
// GPIO38 = WL_CS
// GPIO39 = WL_CLK

// Analog
static const uint8_t A0 = (40u); // ADC0 = GPIO40
static const uint8_t A1 = (41u); // ADC1 = GPIO41`
static const uint8_t A2 = (42u); // ADC2 = GPIO42
static const uint8_t A3 = (43u); // ADC3 = GPIO43
static const uint8_t A4 = (44u); // ADC4 = GPIO44
static const uint8_t A5 = (45u); // ADC5 = GPIO45
static const uint8_t A6 = (46u); // ADC6 = GPIO46 / VSYS_SENSE
