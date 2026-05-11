#pragma once

#include <stdint.h>

#include <cyw43_wrappers.h>

// Pin definitions for the Pimoroni Presto
// https://shop.pimoroni.com/products/presto
// RP2350B (PGA2350) + CYW43 WiFi + 16MB Flash + 8MB PSRAM

#define PINS_COUNT          (48u)
#define NUM_DIGITAL_PINS    (48u)
#define NUM_ANALOG_INPUTS   (8u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

// LED_BUILTIN (accent LED on CYW43 virtual pin)
#define PIN_LED             (64u)
#define LED_BUILTIN         PIN_LED

// Serial
#define PIN_SERIAL1_TX      (0u)
#define PIN_SERIAL1_RX      (1u)

#define PIN_SERIAL2_TX      (8u)
#define PIN_SERIAL2_RX      (9u)

// SPI
#define PIN_SPI0_MISO       (16u)
#define PIN_SPI0_MOSI       (19u)
#define PIN_SPI0_SCK        (18u)
#define PIN_SPI0_SS         (17u)

#define PIN_SPI1_MISO       (12u)
#define PIN_SPI1_MOSI       (15u)
#define PIN_SPI1_SCK        (14u)
#define PIN_SPI1_SS         (13u)

// Wire
#define PIN_WIRE0_SDA       (40u)
#define PIN_WIRE0_SCL       (41u)

#define PIN_WIRE1_SDA       (30u)
#define PIN_WIRE1_SCL       (31u)

#define SERIAL_HOWMANY      (3u)
#define SPI_HOWMANY         (2u)
#define WIRE_HOWMANY        (2u)

// PSRAM
#define RP2350_PSRAM_CS         (47u)
#define RP2350_PSRAM_MAX_SCK_HZ (109*1000*1000)

#define PICO_RP2350A 0 // RP2350B

// All 48 GPIOs available on RP2350B
static const uint8_t D0 = (0u);
static const uint8_t D1 = (1u);
static const uint8_t D2 = (2u);
static const uint8_t D3 = (3u);
static const uint8_t D4 = (4u);
static const uint8_t D5 = (5u);
static const uint8_t D6 = (6u);
static const uint8_t D7 = (7u);
static const uint8_t D8 = (8u);
static const uint8_t D9 = (9u);
static const uint8_t D10 = (10u);
static const uint8_t D11 = (11u);
static const uint8_t D12 = (12u);
static const uint8_t D13 = (13u);
static const uint8_t D14 = (14u);
static const uint8_t D15 = (15u);
static const uint8_t D16 = (16u);
static const uint8_t D17 = (17u);
static const uint8_t D18 = (18u);
static const uint8_t D19 = (19u);
static const uint8_t D20 = (20u);
static const uint8_t D21 = (21u);
static const uint8_t D22 = (22u);
static const uint8_t D23 = (23u);
static const uint8_t D24 = (24u);
static const uint8_t D25 = (25u);
static const uint8_t D26 = (26u);
static const uint8_t D27 = (27u);
static const uint8_t D28 = (28u);
static const uint8_t D29 = (29u);
static const uint8_t D30 = (30u);
static const uint8_t D31 = (31u);
static const uint8_t D32 = (32u);
static const uint8_t D33 = (33u);
static const uint8_t D34 = (34u);
static const uint8_t D35 = (35u);
static const uint8_t D36 = (36u);
static const uint8_t D37 = (37u);
static const uint8_t D38 = (38u);
static const uint8_t D39 = (39u);
static const uint8_t D40 = (40u);
static const uint8_t D41 = (41u);
static const uint8_t D42 = (42u);
static const uint8_t D43 = (43u);
static const uint8_t D44 = (44u);
static const uint8_t D45 = (45u);
static const uint8_t D46 = (46u);
static const uint8_t D47 = (47u);

static const uint8_t A0 = (40u);
static const uint8_t A1 = (41u);
static const uint8_t A2 = (42u);
static const uint8_t A3 = (43u);
static const uint8_t A4 = (44u);
static const uint8_t A5 = (45u);
static const uint8_t A6 = (46u);
static const uint8_t A7 = (47u);

static const uint8_t SS = PIN_SPI0_SS;
static const uint8_t MOSI = PIN_SPI0_MOSI;
static const uint8_t MISO = PIN_SPI0_MISO;
static const uint8_t SCK = PIN_SPI0_SCK;

static const uint8_t SDA = PIN_WIRE0_SDA;
static const uint8_t SCL = PIN_WIRE0_SCL;
