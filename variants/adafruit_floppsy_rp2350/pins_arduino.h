#pragma once

// Floppsy Rev D: RP2350B, 16 MB flash, 8 MB PSRAM.
#define PICO_RP2350A 0

// The power indicator LEDs are not connected to GPIO.
#define PIN_NEOPIXEL (18u)
#define NUM_NEOPIXEL (1u)

#define PIN_USB_HOST_DP (1u)
#define PIN_USB_HOST_DM (2u)
#define PIN_USB_HOST_ENABLE (0u)
#define PIN_5V_EN PIN_USB_HOST_ENABLE
#define PIN_5V_EN_STATE 1

#define TFT_BACKLIGHT (3u)
#define TFT_RESET (4u)
#define TFT_DC (5u)
#define TFT_CS (8u)

#define PIN_SD_DETECT (33u)
#define PIN_SD_CLK (34u)
#define PIN_SD_CMD_MOSI (35u)
#define PIN_SD_DAT0_MISO (36u)
#define PIN_SD_DAT1 (37u)
#define PIN_SD_DAT2 (38u)
#define PIN_SD_DAT3_CS (39u)
#define PIN_SD_CS PIN_SD_DAT3_CS

// Directions as seen from the front of the assembled enclosure.
#define PIN_BUTTON_UP (45u)
#define PIN_BUTTON_RIGHT (46u)
#define PIN_BUTTON_DOWN (43u)
#define PIN_BUTTON_LEFT (44u)
#define PIN_BUTTON_SELECT (42u)
#define PIN_BUTTON_BOOT (32u)

#define FLOPPY_STAT_DIR (9u)
#define FLOPPY_STAT_OE (10u)
#define FLOPPY_READY (11u)
#define FLOPPY_INDEX (12u)
#define FLOPPY_FPC_READY (13u)
#define FLOPPY_FPC_HD (14u)
#define FLOPPY_RDDATA (15u)
#define FLOPPY_TRK0 (16u)
#define FLOPPY_WRPROT (17u)
#define FLOPPY_CMD_DIR (19u)
#define FLOPPY_CMD_OE (20u)
#define FLOPPY_SIDE (21u)
#define FLOPPY_WRDATA (22u)
#define FLOPPY_STEP (23u)
#define FLOPPY_DIR (24u)
#define FLOPPY_MOTOR (25u)
#define FLOPPY_SELECT1 (26u)
#define FLOPPY_SELECT FLOPPY_SELECT1
#define FLOPPY_DENSITY (27u)
#define FLOPPY_SELECT0 (30u)
#define FLOPPY_WRGATE (31u)

// Connector AD0 is ADC1; connector AD1 is ADC0.
#define __PIN_A0 (41u)
#define __PIN_A1 (40u)
#define APPLE2_INDEX __PIN_A0

// No spare UART pins on the populated board.
#define __SERIAL1_DEVICE uart0
#define PIN_SERIAL1_TX (99u)
#define PIN_SERIAL1_RX (99u)
#define __SERIAL2_DEVICE uart1
#define PIN_SERIAL2_TX (99u)
#define PIN_SERIAL2_RX (99u)
#define SERIAL_HOWMANY (0u)

// TFT is write-only SPI0. SDIO has dedicated pins above.
#define __SPI0_DEVICE spi0
#define PIN_SPI0_MISO (255u)
#define PIN_SPI0_MOSI (7u)
#define PIN_SPI0_SCK (6u)
#define PIN_SPI0_SS TFT_CS
#define __SPI1_DEVICE spi1
#define PIN_SPI1_MISO (99u)
#define PIN_SPI1_MOSI (99u)
#define PIN_SPI1_SCK (99u)
#define PIN_SPI1_SS (99u)
#define SPI_HOWMANY (1u)

#define __WIRE0_DEVICE i2c0
#define PIN_WIRE0_SDA (28u)
#define PIN_WIRE0_SCL (29u)
#define __WIRE1_DEVICE i2c1
#define PIN_WIRE1_SDA (99u)
#define PIN_WIRE1_SCL (99u)
#define WIRE_HOWMANY (1u)

#define RP2350_PSRAM_CS (47u)
#define RP2350_PSRAM_MAX_SCK_HZ (84 * 1000 * 1000)

#include "../generic/common.h"
