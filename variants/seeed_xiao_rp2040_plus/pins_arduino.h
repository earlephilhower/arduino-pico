#pragma once

// Pin definitions for XIAO RP2040 Plus
// Derived from schematic: 202004621_XIAO RP2040-Plus_V1.0_SCH&PCB_2604222
// RP2040 (U14) -> XIAO-Add-On-Plus header (U4, 23-pin)

// LEDs
// ----
// User LED on GPIO25
#define PIN_LED              (25u)
#define LED_BUILTIN          PIN_LED

// RGB LED (WS2812B) data pin
#define PIN_NEOPIXEL         (12u)

// RGB LED power enable (active high)
#define NEOPIXEL_POWER       (24u)

// Digital pins
// ------------
// D0-D10: same as old XIAO RP2040 (backward compatible)
static const uint8_t D0  = (26u);  // GPIO26 / A0
static const uint8_t D1  = (27u);  // GPIO27 / A1
static const uint8_t D2  = (28u);  // GPIO28 / A2
static const uint8_t D3  = (29u);  // GPIO29 / A3
static const uint8_t D4  = (6u);   // GPIO6  / SDA0
static const uint8_t D5  = (7u);   // GPIO7  / SCL0
static const uint8_t D6  = (0u);   // GPIO0  / TX
static const uint8_t D7  = (1u);   // GPIO1  / RX
static const uint8_t D8  = (2u);   // GPIO2  / SCK
static const uint8_t D9  = (4u);   // GPIO4  / MISO
static const uint8_t D10 = (3u);   // GPIO3  / MOSI

// D11: not pinned out as GPIO (used internally)

// D12-D27: new pins on XIAO RP2040 Plus
static const uint8_t D12 = (18u);  // GPIO18 / SDA1
static const uint8_t D13 = (19u);  // GPIO19 / SCL1
static const uint8_t D14 = (20u);  // GPIO20
static const uint8_t D15 = (21u);  // GPIO21
static const uint8_t D16 = (22u);  // GPIO22
static const uint8_t D17 = (23u);  // GPIO23
// D18: not defined on this schematic
static const uint8_t D19 = (5u);   // GPIO5
static const uint8_t D20 = (13u);  // GPIO13
static const uint8_t D21 = (14u);  // GPIO14
static const uint8_t D22 = (15u);  // GPIO15
static const uint8_t D23 = (16u);  // GPIO16
static const uint8_t D24 = (17u);  // GPIO17
static const uint8_t D25 = (10u);  // GPIO10
static const uint8_t D26 = (9u);   // GPIO9
static const uint8_t D27 = (8u);   // GPIO8

// Analog pins
// -----------
static const uint8_t A0  = (26u);  // GPIO26
static const uint8_t A1  = (27u);  // GPIO27
static const uint8_t A2  = (28u);  // GPIO28
static const uint8_t A3  = (29u);  // GPIO29
#define ADC_RESOLUTION 12

// Serial
// ------
#define PIN_SERIAL1_TX (0u)   // GPIO0 / D6
#define PIN_SERIAL1_RX (1u)   // GPIO1 / D7

// Serial2 not pinned out
#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI0
// ----
#define PIN_SPI0_MISO  (4u)   // GPIO4 / D9
#define PIN_SPI0_MOSI  (3u)   // GPIO3 / D10
#define PIN_SPI0_SCK   (2u)   // GPIO2 / D8
#define PIN_SPI0_SS    (31u)  // not pinned out
static const uint8_t SS   = PIN_SPI0_SS;
static const uint8_t MOSI = PIN_SPI0_MOSI;
static const uint8_t MISO = PIN_SPI0_MISO;
static const uint8_t SCK  = PIN_SPI0_SCK;

// SPI1 not pinned out
#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)
#define SPI_MISO       (PIN_SPI1_MISO)
#define SPI_MOSI       (PIN_SPI1_MOSI)
#define SPI_SCK        (PIN_SPI1_SCK)

// Wire0 - I2C0 on i2c1
// ---------------------
#define __WIRE0_DEVICE  (i2c1)
#define PIN_WIRE0_SDA   (6u)   // GPIO6 / D4
#define PIN_WIRE0_SCL   (7u)   // GPIO7 / D5
#define SDA             PIN_WIRE0_SDA
#define SCL             PIN_WIRE0_SCL
#define I2C_SDA         (SDA)
#define I2C_SCL         (SCL)

// Wire1 - I2C1 on i2c0 (NEW on Plus)
// ------------------------------------
#define __WIRE1_DEVICE  (i2c0)
#define PIN_WIRE1_SDA   (20u)  // GPIO20 / D14
#define PIN_WIRE1_SCL   (21u)  // GPIO21 / D15

// Interface counts
#define SERIAL_HOWMANY  (1u)
#define SPI_HOWMANY     (1u)
#define WIRE_HOWMANY    (2u)
