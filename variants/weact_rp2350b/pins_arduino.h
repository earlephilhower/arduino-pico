#pragma once

// WeAct Studio RP2350B Core Board 
// https://github.com/WeActStudio/WeActStudio.RP2350BCoreBoard
// RP2350B: 48 GPIO pins (GP0-GP47), ADC on GP40-GP47 (Channels 0-7) 
// Pin layout: even pins on the inner row, odd pins on the outer row 
// IMPORTANT: PICO_RP2350A must be 0 to activate the B-Die branch (48 GPIOs) in 
// ../generic/common.h. common.h then automatically defines 
// D0-D47 and A0-A7 (A0=GP40 ... A7=GP47) suitable for the WeAct pinout 

#define PICO_RP2350A 0 // RP2350B

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)
#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
#define PIN_SPI0_MISO  (16u)
#define PIN_SPI0_MOSI  (19u)
#define PIN_SPI0_SCK   (18u)
#define PIN_SPI0_SS    (17u)
#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (15u)
#define PIN_SPI1_SCK   (14u)
#define PIN_SPI1_SS    (13u)

// Wire (I2C)
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)
#define PIN_WIRE1_SDA  (26u)
#define PIN_WIRE1_SCL  (27u)

// LED
#define PIN_LED        (25u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"


