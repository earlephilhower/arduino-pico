#pragma once

#define PINS_COUNT          (28u)
#define NUM_DIGITAL_PINS    (28u)
#define NUM_ANALOG_INPUTS   (4u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

// LEDs
#define PIN_LED             (19u)

// Serial1 (User serial port)
#define PIN_SERIAL1_TX      (16u)
#define PIN_SERIAL1_RX      (17u)

// Serial2, connected to SARA-R4XX modem
#define PIN_SERIAL2_TX      (4u)
#define PIN_SERIAL2_RX      (5u)
#define PIN_SERIAL2_CTS     (6u)
#define PIN_SERIAL2_RTS     (7u)
#define PIN_SARA_ON         (13u)
#define PIN_SARA_RST        (14u)
#define PIN_SARA_PWR        (15u)
#define SARA_SERIAL_PORT    Serial2

// SPI
#define PIN_SPI0_MISO       (20u)
#define PIN_SPI0_MOSI       (23u)
#define PIN_SPI0_SCK        (22u)
#define PIN_SPI0_SS         (21u)

// Not pinned out
#define PIN_SPI1_MISO       (31u)
#define PIN_SPI1_MOSI       (31u)
#define PIN_SPI1_SCK        (31u)
#define PIN_SPI1_SS         (31u)

// Wire
#define PIN_WIRE0_SDA       (0u)
#define PIN_WIRE0_SCL       (1u)

// Not pinned out
#define PIN_WIRE1_SDA       (31u)
#define PIN_WIRE1_SCL       (31u)

#define SERIAL_HOWMANY      (2u)
#define SPI_HOWMANY         (1u)
#define WIRE_HOWMANY        (1u)

#define LED_BUILTIN         PIN_LED

static const uint8_t D0 = (16u);
static const uint8_t D1 = (17u);
static const uint8_t D2 = (24u);
static const uint8_t D3 = (23u);
static const uint8_t D4 = (22u);
static const uint8_t D5 = (2u);
static const uint8_t D6 = (3u);
static const uint8_t D7 = (0u);
static const uint8_t D8 = (1u);
static const uint8_t D9 = (12u);
static const uint8_t D10 = (11u);
static const uint8_t D11 = (10u);
static const uint8_t D12 = (9u);
static const uint8_t D13 = (8u);
static const uint8_t D14 = (13u);
static const uint8_t D15 = (14u);
static const uint8_t D16 = (15u);
static const uint8_t D17 = (19u);

static const uint8_t A0 = (29u);
static const uint8_t A1 = (28u);
static const uint8_t A2 = (27u);
static const uint8_t A3 = (26u);
static const uint8_t A4 = (25u);
static const uint8_t A5 = (21u);
