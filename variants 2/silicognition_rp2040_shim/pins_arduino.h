#pragma once

// Pin definitions taken from:
//    https://silicognition.com/Products/rp2040-shim/

// LEDs
#define LED_BUILTIN    (22u)

// NeoPixel
#define PIN_NEOPIXEL   (23u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

// Not pinned out
#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)


// SPI0
#define __SPI0_DEVICE  spi1
#define PIN_SPI0_MISO  (12u)
#define PIN_SPI0_MOSI  (11u)
#define PIN_SPI0_SCK   (10u)
#define PIN_SPI0_SS    (21u)

// SPI1
#define __SPI1_DEVICE  spi0
#define PIN_SPI1_MISO  (20u)
#define PIN_SPI1_MOSI  (19u)
#define PIN_SPI1_SCK   (18u)
#define PIN_SPI1_SS    (15u)

// Wire
#define __WIRE0_DEVICE i2c0
#define PIN_WIRE0_SDA  (16u)
#define PIN_WIRE0_SCL  (17u)
#define __WIRE1_DEVICE i2c1
#define PIN_WIRE1_SDA  (31u)
#define PIN_WIRE1_SCL  (31u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (1u)

// Defines normally loaded from common.h
// We have custom pin mapping to match Adafruit Feather

#define PINS_COUNT          (30u)
#define NUM_DIGITAL_PINS    (30u)
#define NUM_ANALOG_INPUTS   (4u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

static const uint8_t D0 = (1u);
static const uint8_t D1 = (0u);
static const uint8_t D4 = (6u);
static const uint8_t D5 = (18u);
static const uint8_t D6 = (19u);
static const uint8_t D9 = (20u);
static const uint8_t D10 = (21u);
static const uint8_t D11 = (15u);
static const uint8_t D12 = (14u);
static const uint8_t D13 = (22u);
static const uint8_t D24 = (24u);
static const uint8_t D25 = (25u);

static const uint8_t A0 = (29u);
static const uint8_t A1 = (28u);
static const uint8_t A2 = (27u);
static const uint8_t A3 = (26u);

static const uint8_t SS = PIN_SPI0_SS;
static const uint8_t MOSI = PIN_SPI0_MOSI;
static const uint8_t MISO = PIN_SPI0_MISO;
static const uint8_t SCK = PIN_SPI0_SCK;

static const uint8_t SDA = PIN_WIRE0_SDA;
static const uint8_t SCL = PIN_WIRE0_SCL;

