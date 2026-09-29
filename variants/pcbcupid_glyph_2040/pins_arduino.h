/*
    Copyright (c) 2024 PCBCupid
    SPDX-License-Identifier: MIT
*/
#pragma once

// LED
#define PIN_LED         (0u)

// Serial1 (UART0)
#define PIN_SERIAL1_TX  (12u)
#define PIN_SERIAL1_RX  (13u)

// Serial2 (UART1)
#define PIN_SERIAL2_TX  (8u)
#define PIN_SERIAL2_RX  (9u)

// SPI0
#define PIN_SPI0_SCK    (6u)
#define PIN_SPI0_MOSI   (7u)
#define PIN_SPI0_MISO   (8u)
#define PIN_SPI0_SS     (5u)

// SPI1 — not pinned out
#define PIN_SPI1_MISO   (31u)
#define PIN_SPI1_MOSI   (31u)
#define PIN_SPI1_SCK    (31u)
#define PIN_SPI1_SS     (31u)

// Wire (I2C0)
#define __WIRE0_DEVICE  i2c0
#define PIN_WIRE0_SDA   (20u)
#define PIN_WIRE0_SCL   (21u)

// ADC
#define PIN_A0          (26u)
#define PIN_A1          (27u)
#define PIN_A2          (28u)
#define PIN_A3          (29u)

#define PINS_COUNT          (30u)
#define NUM_DIGITAL_PINS    (30u)
#define NUM_ANALOG_INPUTS   (4u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

#define SERIAL_HOWMANY  (2u)
#define SPI_HOWMANY     (1u)
#define WIRE_HOWMANY    (1u)

#include "../generic/common.h"
