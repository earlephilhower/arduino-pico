#pragma once

// Pin definitions taken from:
// https://github.com/rp-rs/rp-hal-boards/blob/main/boards/pimoroni-plasma-2040/src/lib.rs

// LEDs
#define PIN_LED        (16u)
#define PIN_LED_R      (16u)
#define PIN_LED_G      (17u)
#define PIN_LED_B      (18u)
#define LED_BUILTIN    PIN_LED

// Digital pins
#if 0
static const uint8_t D0 = (26u);
static const uint8_t D1 = (27u);
static const uint8_t D2 = (28u);
static const uint8_t D3 = (29u);
static const uint8_t D4 = (6u);
static const uint8_t D5 = (7u);
static const uint8_t D6 = (0u);
static const uint8_t D7 = (1u);
static const uint8_t D8 = (2u);
static const uint8_t D9 = (4u);
static const uint8_t D10 = (3u);
#endif

// Analog pins
static const uint8_t A0  = (26u);
static const uint8_t A1  = (27u);
static const uint8_t A2  = (28u);
static const uint8_t A3  = (31u);
#define ADC_RESOLUTION 12

// NeoPixel
#define PIN_NEOPIXEL   (15u)
//#define NEOPIXEL_POWER (11u)

// Serial1
#define PIN_SERIAL1_TX (31u)
#define PIN_SERIAL1_RX (31u)

// Serial2 not pinned out
#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI0_MISO  (31u)
#define PIN_SPI0_MOSI  (31u)
#define PIN_SPI0_SCK   (31u)
#define PIN_SPI0_SS    (31u) // not pinned out
//static const uint8_t SS   = PIN_SPI0_SS;   // SPI Slave SS not used. Set here only for reference.
//static const uint8_t MOSI = PIN_SPI0_MOSI;
//static const uint8_t MISO = PIN_SPI0_MISO;
//static const uint8_t SCK  = PIN_SPI0_SCK;

// Not pinned out
#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)
//#define SPI_MISO       (PIN_SPI1_MISO)
//#define SPI_MOSI       (PIN_SPI1_MOSI)
//#define SPI_SCK        (PIN_SPI1_SCK)

// Wire
#define __WIRE0_DEVICE (i2c1)
#define PIN_WIRE0_SDA  (20u)
#define PIN_WIRE0_SCL  (21u)
#define SDA            PIN_WIRE0_SDA
#define SCL            PIN_WIRE0_SCL
#define I2C_SDA        (SDA)
#define I2C_SCL        (SCL)

// Wire1 not pinned out
#define __WIRE1_DEVICE (i2c0)
#define PIN_WIRE1_SDA  (31u)
#define PIN_WIRE1_SCL  (31u)

#define SERIAL_HOWMANY (0u)
#define SPI_HOWMANY    (0u)
#define WIRE_HOWMANY   (1u)
