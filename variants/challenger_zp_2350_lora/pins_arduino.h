#pragma once

#define PICO_RP2350A 1

#define PINS_COUNT          (30u)
#define NUM_DIGITAL_PINS    (30u)
#define NUM_ANALOG_INPUTS   (4u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

// LEDs
#define PIN_LED             (15u)

// Serial
#define PIN_SERIAL1_TX      (12u)
#define PIN_SERIAL1_RX      (13u)

// SPI0
#define PIN_SPI0_MISO       (16u)
#define PIN_SPI0_SS         (17u)
#define PIN_SPI0_SCK        (18u)
#define PIN_SPI0_MOSI       (19u)

// SPI1 Connected to SX1262 LoRa chip
#define PIN_SPI1_MISO       (8u)
#define PIN_SPI1_MOSI       (11u)
#define PIN_SPI1_SCK        (10u)
#define PIN_SPI1_SS         (9u)

#define PIN_SX1262_SS       PIN_SPI1_SS
#define PIN_SX1262_DIO1     (7u)
#define PIN_SX1262_BUSY     (6u)
#define PIN_SX1262_RSTN     (5u)
#define SX1262_SPI          SPI1

// Wire
#define PIN_WIRE0_SDA       (20u)
#define PIN_WIRE0_SCL       (21u)

// iProbe debug interface pin
#define PIN_IPROBE_TX       (14u)

// Not pinned out
#define PIN_WIRE1_SDA       (31u)
#define PIN_WIRE1_SCL       (31u)
#define PIN_SERIAL2_RX      (31u)
#define PIN_SERIAL2_TX      (31u)

#define SERIAL_HOWMANY      (1u)
#define SPI_HOWMANY         (2u)
#define WIRE_HOWMANY        (1u)

#define LED_BUILTIN         PIN_LED

static const uint8_t D0 = (12u);
static const uint8_t D1 = (13u);
static const uint8_t D5 = (23u);
static const uint8_t D6 = (24u);
static const uint8_t D9 = (25u);
static const uint8_t D10 = (1u);
static const uint8_t D11 = (2u);
static const uint8_t D12 = (3u);
static const uint8_t D13 = (4u);

static const uint8_t A0 = (29u);
static const uint8_t A1 = (28u);
static const uint8_t A2 = (27u);
static const uint8_t A3 = (26u);
static const uint8_t A4 = (1u);

static const uint8_t A5 = PIN_SPI0_SS;

static const uint8_t SS = PIN_SPI0_SS;
static const uint8_t MOSI = PIN_SPI0_MOSI;
static const uint8_t MISO = PIN_SPI0_MISO;
static const uint8_t SCK = PIN_SPI0_SCK;

static const uint8_t SDA = PIN_WIRE0_SDA;
static const uint8_t SCL = PIN_WIRE0_SCL;

static const uint8_t RX = PIN_SERIAL1_RX;
static const uint8_t TX = PIN_SERIAL1_TX;
