#pragma once

#define PICO_RP2350A 0 // RP2350B

// LEDs
#define PIN_LED        (23u)

#define PIN_NEOPIXEL   (25)
#define NUM_NEOPIXEL   (1)

// 'Boot0' button also on GPIO #24
#define PIN_BUTTON      (24u)

// USB host connector
#define PIN_USB_HOST_DP (32u)
#define PIN_USB_HOST_DM (33u)
#define PIN_5V_EN       (29u)
#define PIN_5V_EN_STATE (1u)

// SDIO
#define PIN_SD_CLK       (34u)
#define PIN_SD_CMD_MOSI  (35u)
#define PIN_SD_DAT0_MISO (36u)
#define PIN_SD_DAT1      (37u)
#define PIN_SD_DAT2      (38u)
#define PIN_SD_DAT3_CS   (39u)
#define PIN_SD_DETECT    (40u)

#define __PIN_A0        (41u)
#define __PIN_A1        (42u)
#define __PIN_A2        (43u)
#define __PIN_A3        (44u)
#define __PIN_A4        (45u)
#define __PIN_A5        (46u)

// UARTs
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)
#define PIN_SERIAL2_TX (99u) // not pinned out
#define PIN_SERIAL2_RX (99u)

// SPI
#define __SPI0_DEVICE   spi1
#define PIN_SPI1_MISO  (36u)
#define PIN_SPI1_MOSI  (35u)
#define PIN_SPI1_SCK   (34u)
#define PIN_SPI1_SS    (39u)

#define __SPI1_DEVICE   spi0
#define PIN_SPI0_MISO  (28u)
#define PIN_SPI0_MOSI  (31u)
#define PIN_SPI0_SCK   (30u)
#define PIN_SPI0_SS    (29u)

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
#define PIN_CKN (15u)
#define PIN_CKP (14u)
#define PIN_D0N (19u)
#define PIN_D0P (18u)
#define PIN_D1N (17u)
#define PIN_D1P (16u)
#define PIN_D2N (13u)
#define PIN_D2P (12u)


#include "../generic/common.h"
