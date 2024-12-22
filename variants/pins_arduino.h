#pragma once

// Pin definitions taken from:
//    https://datasheets.raspberrypi.org/pico/pico-datasheet.pdf


// LEDs not pinned out
#define PIN_LED        (31u)

// NeoPixel
#define PIN_NEOPIXEL   (4u)
#define NEOPIXEL_POWER (31u)
// Serial
#define PIN_SERIAL1_TX (28u)
#define PIN_SERIAL1_RX (29u)
/*
#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)
*/
/**#define TFT_MOSI 6  // Data out
  #define TFT_SCLK 8  // Clock out
  #define TFT_CS         9
  #define TFT_RST        -1 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC         5

  Adafruit_ST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst = -1);

Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
 Adafruit_ST77xx(uint16_t w, uint16_t h, int8_t _CS, int8_t _DC, int8_t _MOSI,
                  int8_t _SCLK, int8_t _RST = -1, int8_t _MISO = -1);


GND : Power Ground
VCC : Power input
CS : Chipselect
SCK/SCLK (SD-Clock): SPI Clock
MOSI (SD-DI, DI) : SPI Master out Slave in
MISO (SD-DO, DO) : SPI Master in Slave out
CD: Card Detect (see comment of rollinger below (thanks).

There are several alternative names for the same signal:
DC Data / Command
RS Register Select
A0 Address Select


 */
// SPI

/** LCD ST7789 SPI */
#define PIN_SPI0_MISO  (31u) //doesnt have one
#define PIN_SPI0_MOSI  (6u)
#define PIN_SPI0_SCK   (8u)
#define PIN_SPI0_SS    (9u) //using as CS

//SD card SPI
#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (15u)
#define PIN_SPI1_SCK   (14u)
#define PIN_SPI1_SS    (13u)

// Wire (I2C gryo + RTC)
#define PIN_WIRE0_SDA  (2u)
#define PIN_WIRE0_SCL  (3u)

/*
#define PIN_WIRE1_SDA  (26u)
#define PIN_WIRE1_SCL  (27u)
*/
#define SERIAL_HOWMANY (1u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (1u)

#include "../generic/common.h"

/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// -----------------------------------------------------
// NOTE: THIS HEADER IS ALSO INCLUDED BY ASSEMBLER SO
//       SHOULD ONLY CONSIST OF PREPROCESSOR DIRECTIVES
// -----------------------------------------------------

// pico_cmake_set PICO_PLATFORM=rp2350

#ifndef _BOARDS_DEFCON32_BADGE_H
#define _BOARDS_DEFCON32_BADGE_H

// For board detection
#define DEFCON32_BADGE

// --- RP2350 VARIANT ---
#define PICO_RP2350A 1

#define DEFCON32_BADGE_SRAM_CS_PIN             0 // PSRAM not actually populated on this PCB
#define DEFCON32_BADGE_TOUCH_INT_PIN           1 //ns2009 driver for touchscreen

#define DEFCON32_BADGE_I2C_SDA_PIN             2
#define DEFCON32_BADGE_I2C_SDL_PIN             3

#define DEFCON32_BADGE_WS2812_PIN              4 //neopixels my belovid

#define DEFCON32_BADGE_DISPLAY_RS_PIN          5 //aka dc
#define DEFCON32_BADGE_DISPLAY_DO_PIN          6 //MOSI
#define DEFCON32_BADGE_IR_SD_PIN               7 //something with the IR reciver
#define DEFCON32_BADGE_DISPLAY_SCK_PIN         8 //sclk / clk clock
#define DEFCON32_BADGE_DISPLAY_CS_PIN          9 //aka SS
#define DEFCON32_BADGE_DISPLAY_BL_PIN          10 //backlight

//no idea
#define DEFCON32_BADGE_SYS_POWER_CONTROL_PIN   11

//SD card pins
#define DEFCON32_BADGE_SPI_MISO_PIN            12
#define DEFCON32_BADGE_SD_CS_PIN               13 //aka ss
#define DEFCON32_BADGE_SPI_CK_PIN              14
#define DEFCON32_BADGE_SPI_MOSI_PIN            15

//buttons
#define DEFCON32_BADGE_SW_RIGHT_PIN            16
#define DEFCON32_BADGE_SW_DOWN_PIN             17
#define DEFCON32_BADGE_SW_UP_PIN               18
#define DEFCON32_BADGE_SW_LEFT_PIN             19
#define DEFCON32_BADGE_SW_B_PIN                20
#define DEFCON32_BADGE_SW_A_PIN                21
#define DEFCON32_BADGE_SW_START_PIN            22
#define DEFCON32_BADGE_SW_SELECT_PIN           23
#define DEFCON32_BADGE_SW_FN_PIN               24


#define DEFCON32_BADGE_SPEAKER_OUT_PIN         25

#define DEFCON32_BADGE_IR_RX_PIN               26
#define DEFCON32_BADGE_IR_TX_PIN               27

// --- UART ---
// NOTE: since there is no UART on the badge, you should probably pass:
// -DPICO_BOARD=defcon32_badge -DPICO_STDIO_USB=1 -DPICO_STDIO_UART=0
// when building to set up stdio over USB CDC by default

// --- LED ---
// no PICO_DEFAULT_LED_PIN
#ifndef PICO_DEFAULT_WS2812_PIN
#define PICO_DEFAULT_WS2812_PIN DEFCON32_BADGE_WS2812_PIN
#endif

// --- I2C ---
#ifndef PICO_DEFAULT_I2C
#define PICO_DEFAULT_I2C 1
#endif
#ifndef PICO_DEFAULT_I2C_SDA_PIN
#define PICO_DEFAULT_I2C_SDA_PIN DEFCON32_BADGE_I2C_SDA_PIN
#endif
#ifndef PICO_DEFAULT_I2C_SCL_PIN
#define PICO_DEFAULT_I2C_SCL_PIN DEFCON32_BADGE_I2C_SDL_PIN
#endif

// --- SPI ---
#ifndef PICO_DEFAULT_SPI
#define PICO_DEFAULT_SPI 1
#endif
#ifndef PICO_DEFAULT_SPI_SCK_PIN
#define PICO_DEFAULT_SPI_SCK_PIN DEFCON32_BADGE_SPI_CK_PIN
#endif
#ifndef PICO_DEFAULT_SPI_TX_PIN
#define PICO_DEFAULT_SPI_TX_PIN DEFCON32_BADGE_SPI_MOSI_PIN
#endif
#ifndef PICO_DEFAULT_SPI_RX_PIN
#define PICO_DEFAULT_SPI_RX_PIN DEFCON32_BADGE_SPI_MISO_PIN
#endif
// multiple devices, so this doesn't make much sense
// no PICO_DEFAULT_SPI_CSN_PIN

#ifndef PICO_AUDIO_PWM_L_PIN
#define PICO_AUDIO_PWM_L_PIN DEFCON32_BADGE_SPEAKER_OUT_PIN
#endif

#ifndef PICO_AUDIO_PWM_MONO_PIN
#define PICO_AUDIO_PWM_MONO_PIN PICO_AUDIO_PWM_L_PIN
#endif

// --- FLASH ---

#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1

#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2
#endif

// pico_cmake_set_default PICO_FLASH_SIZE_BYTES = (4 * 1024 * 1024)
#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (4 * 1024 * 1024)
#endif

// pico_cmake_set_default PICO_RP2350_A2_SUPPORTED = 1
#ifndef PICO_RP2350_A2_SUPPORTED
#define PICO_RP2350_A2_SUPPORTED 1
#endif

#endif
