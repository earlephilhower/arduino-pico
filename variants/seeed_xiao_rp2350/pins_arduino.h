#pragma once

#define PICO_RP2350A 1

// Pin definitions taken from:
//    https://www.seeedstudio.com/Seeed-XIAO-RP2350-p-5944.html

static const uint8_t A0 = (26u);
static const uint8_t A1 = (27u);
static const uint8_t A2 = (28u);

static const uint8_t D0 = (26u);
static const uint8_t D1 = (27u);
static const uint8_t D2 = (28u);
static const uint8_t D3 = (5u);
static const uint8_t D4 = (6u);
static const uint8_t D5 = (7u);
static const uint8_t D6 = (0u);
static const uint8_t D7 = (1u);
static const uint8_t D8 = (2u);
static const uint8_t D9 = (4u);
static const uint8_t D10 = (3u);
static const uint8_t D11 = (21u);
static const uint8_t D12 = (20u);
static const uint8_t D13 = (17u);
static const uint8_t D14 = (16u);
static const uint8_t D15 = (11u);
static const uint8_t D16 = (12u);
static const uint8_t D17 = (10u);
static const uint8_t D18 = (9u);

// LEDs
#define PIN_LED        (25u)
#define LED_BUILTIN     PIN_LED

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)


#define PIN_SERIAL2_TX (20u)
#define PIN_SERIAL2_RX (21u)


// SPI
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (5u)


#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (9u)


// Wire
#define __WIRE0_DEVICE (i2c0)
#define PIN_WIRE0_SDA  (16u)
#define PIN_WIRE0_SCL  (17u)
#define SDA            PIN_WIRE0_SDA
#define SCL            PIN_WIRE0_SCL
#define I2C_SDA        (SDA)
#define I2C_SCL        (SCL)

#define __WIRE1_DEVICE (i2c1)
#define PIN_WIRE1_SDA  (6u)
#define PIN_WIRE1_SCL  (7u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)


static const uint8_t MOSI = PIN_SPI0_MOSI;
static const uint8_t MISO = PIN_SPI0_MISO;
static const uint8_t SCK  = PIN_SPI0_SCK;
static const uint8_t SS   = PIN_SPI0_SS;

