#pragma once
#define PICO_RP2350A 0

// LEDs
#define PIN_LED        (29u)

#define PIN_NEOPIXEL   (32u)
#define NUM_NEOPIXEL   (5u)

// 'Boot0' button also on GPIO #0
#define PIN_BUTTON      (0u)
#define PIN_BUTTON1     (4u)
#define PIN_BUTTON2     (5u)

// USB host connector
#define PIN_USB_HOST_DP (1u)
#define PIN_USB_HOST_DM (2u)
#define PIN_5V_EN       (11u)
#define PIN_5V_EN_STATE (1u)

// SDIO
#define PIN_SD_DETECT    (33u)
#define PIN_SD_CLK       (34u)
#define PIN_SD_CMD_MOSI  (35u)
#define PIN_SD_DAT0_MISO (36u)
#define PIN_SD_DAT1      (37u)
#define PIN_SD_DAT2      (38u)
#define PIN_SD_DAT3_CS   (39u)

// I2S
#define PIN_I2S_DATAOUT  (24u)
#define PIN_I2S_WORDSEL  (25u)
#define PIN_I2S_BITCLK   (26u)
#define PIN_I2S_MCLK     (27u)

#define PIN_PERIPHERAL_RESET (22u)

#define __PIN_A0        (40u)
#define __PIN_A1        (41u)
#define __PIN_A2        (42u)
#define __PIN_A3        (43u)
#define __PIN_A4        (44u)
#define __PIN_A5        (45u)

// UARTs
#define PIN_SERIAL1_TX (8u)
#define PIN_SERIAL1_RX (9u)
#define PIN_SERIAL2_TX (99u) // not pinned out
#define PIN_SERIAL2_RX (99u)

// SPI
#define __SPI1_DEVICE   spi1
#define PIN_SPI1_MISO  (28u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (30u)
#define PIN_SPI1_SS    (46u)

#define __SPI0_DEVICE   spi0
#define PIN_SPI0_MISO  (36u)
#define PIN_SPI0_MOSI  (35u)
#define PIN_SPI0_SCK   (34u)
#define PIN_SPI0_SS    (39u)

// Wire
#define __WIRE0_DEVICE i2c0
#define PIN_WIRE0_SDA  (20u)
#define PIN_WIRE0_SCL  (21u)

#define __WIRE1_DEVICE i2c1
#define PIN_WIRE1_SDA  (99u) // not pinned out
#define PIN_WIRE1_SCL  (99u)

#define SERIAL_HOWMANY (1u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (1u)

// PSRAM
#define RP2350_PSRAM_CS         (47u)
#define RP2350_PSRAM_MAX_SCK_HZ (109*1000*1000)

// DVI connector
#define PIN_CKN (12u)
#define PIN_CKP (13u)
#define PIN_D0N (14u)
#define PIN_D0P (15u)
#define PIN_D1N (16u)
#define PIN_D1P (17u)
#define PIN_D2N (18u)
#define PIN_D2P (19u)

#include "../generic/common.h"
