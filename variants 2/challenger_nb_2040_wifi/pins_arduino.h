#pragma once

#define PINS_COUNT          (24u)
#define NUM_DIGITAL_PINS    (24u)
#define NUM_ANALOG_INPUTS   (4u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

// LEDs
#define PIN_LED             (12u)

// Serial
#define PIN_SERIAL1_TX      (16u)
#define PIN_SERIAL1_RX      (17u)

// Connected to ESP8285
#define PIN_SERIAL2_TX      (4u)
#define PIN_SERIAL2_RX      (5u)
#define PIN_ESP8285_RST     (19u)
#define PIN_ESP8285_MODE    (13u)
#define ESP8285_SERIAL      Serial2
// Uart define esp serial abstraction pins
#define PIN_ESP_TX          PIN_SERIAL2_TX
#define PIN_ESP_RX          PIN_SERIAL2_RX
#define PIN_ESP_RST         PIN_ESP8285_RST
#define PIN_ESP_MODE        PIN_ESP8285_MODE
#define ESP_SERIAL_PORT     ESP8285_SERIAL

// SPI
#define PIN_SPI0_MISO       (24u)
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
#define NEOPIXEL            (11u)

static const uint8_t D0 = (16u);
static const uint8_t D1 = (17u);
static const uint8_t D2 = (24u);
static const uint8_t D3 = (23u);
static const uint8_t D4 = (22u);
static const uint8_t D5 = (2u);
static const uint8_t D6 = (3u);
static const uint8_t D7 = (0u);
static const uint8_t D8 = (1u);
static const uint8_t D9 = (6u);
static const uint8_t D10 = (7u);
static const uint8_t D11 = (8u);
static const uint8_t D12 = (9u);
static const uint8_t D13 = (10u);
static const uint8_t D14 = (14u);
static const uint8_t D15 = (15u);
static const uint8_t D16 = (18u);
static const uint8_t D17 = (20u);

static const uint8_t A0 = (26u);
static const uint8_t A1 = (27u);
static const uint8_t A2 = (28u);
static const uint8_t A3 = (29u);
static const uint8_t A4 = (25u);
static const uint8_t A5 = (21u);
