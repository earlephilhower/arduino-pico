#pragma once

// LEDs
#define PIN_LED             (19u)

// Serial1, (UART0) Connected to ESP32 chip)
#define PIN_SERIAL1_TX      (16u)
#define PIN_SERIAL1_RX      (17u)
#define PIN_ESP32_RST       (24u)
#define PIN_ESP32_MODE      (25u)
#define ESP32_SERIAL        Serial1
// Uart define esp serial abstraction pins
#define PIN_ESP_TX          PIN_SERIAL1_TX
#define PIN_ESP_RX          PIN_SERIAL1_RX
#define PIN_ESP_RST         PIN_ESP32_RST
#define PIN_ESP_MODE        PIN_ESP32_MODE
#define ESP_SERIAL_PORT     ESP32_SERIAL

// Serial2, (UART1) connected to SARA-R4XX modem
#define PIN_SERIAL2_TX      (4u)
#define PIN_SERIAL2_RX      (5u)
#define PIN_SERIAL2_CTS     (6u)
#define PIN_SERIAL2_RTS     (7u)
#define PIN_SARA_DTR        (12u)
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

#include "../generic/common.h"

