#pragma once

#define PICO_RP2350A 1

#define PINS_COUNT          (30u)
#define NUM_DIGITAL_PINS    (30u)
#define NUM_ANALOG_INPUTS   (4u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

// LEDs
#define PIN_LED             (7u)

// Board serial port (Uart0)
#define PIN_SERIAL1_TX      (12u)
#define PIN_SERIAL1_RX      (13u)

// Uart connected to ESP32-C6-MINI-1 (Uart1)
#define PIN_SERIAL2_TX      (4u)
#define PIN_SERIAL2_RX      (5u)
#define PIN_ESP32_RST       (15u)
#define PIN_ESP32_MODE      (14u)
#define ESP32_SERIAL        Serial2
// Uart define esp serial abstraction pins
#define PIN_ESP_TX          PIN_SERIAL2_TX
#define PIN_ESP_RX          PIN_SERIAL2_RX
#define PIN_ESP_RST         PIN_ESP32_RST
#define PIN_ESP_MODE        PIN_ESP32_MODE
#define ESP_SERIAL_PORT     ESP32_SERIAL

// SPI
#define PIN_SPI0_MISO       (16u)
#define PIN_SPI0_MOSI       (19u)
#define PIN_SPI0_SCK        (18u)
#define PIN_SPI0_SS         (17u)

// SPI connected to ESP32-C6-MINI-1
#define PIN_SPI1_MISO       (8u)
#define PIN_SPI1_MOSI       (11u)
#define PIN_SPI1_SCK        (10u)
#define PIN_SPI1_SS         (9u)
// Handshake signal from ESP32C6
#define PIN_ESP32_HS        (22u)
#define ESP32_SPI           SPI1
// Definitions for ESP hosted
#define ESPHOST_DATA_READY  PIN_ESP_MODE
#define ESPHOST_RESET       PIN_ESP_RST
#define ESPHOST_HANDSHAKE   PIN_ESP32_HS
#define ESPHOST_CS          PIN_SPI1_SS

// Wire
#define PIN_WIRE0_SDA       (20u)
#define PIN_WIRE0_SCL       (21u)

// Not pinned out
#define PIN_WIRE1_SDA       (31u)
#define PIN_WIRE1_SCL       (31u)

#define SERIAL_HOWMANY      (2u)
#define SPI_HOWMANY         (2u)
#define WIRE_HOWMANY        (1u)

#define LED_BUILTIN         PIN_LED

// Enables external PSRAM
#define RP2350_PSRAM_CS    0

/* Pins mappings for marked pins on the board */
static const uint8_t D0 = (13u);
static const uint8_t D1 = (12u);
static const uint8_t D5 = (23u);
static const uint8_t D6 = (24u);
static const uint8_t D9 = (25u);
static const uint8_t D10 = (2u);
static const uint8_t D11 = (3u);
static const uint8_t D12 = (6u);
static const uint8_t D13 = (7u);

static const uint8_t A0 = (29u);
static const uint8_t A1 = (28u);
static const uint8_t A2 = (27u);
static const uint8_t A3 = (26u);
static const uint8_t A4 = (1u);
static const uint8_t A5 = (PIN_SPI0_SS);

static const uint8_t SS = PIN_SPI0_SS;
static const uint8_t MOSI = PIN_SPI0_MOSI;
static const uint8_t MISO = PIN_SPI0_MISO;
static const uint8_t SCK = PIN_SPI0_SCK;

static const uint8_t SDA = PIN_WIRE0_SDA;
static const uint8_t SCL = PIN_WIRE0_SCL;

static const uint8_t RX = PIN_SERIAL1_RX;
static const uint8_t TX = PIN_SERIAL1_TX;
