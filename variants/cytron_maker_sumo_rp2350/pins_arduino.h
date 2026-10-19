#pragma once

#define PICO_RP2350A 1

// LEDs
#define PIN_LED             (3u)

// Buttons / switches
#define PIN_BUTTON          (2u)    // USR_BTN / START, active LOW
#define PIN_DIP1            (4u)
#define PIN_DIP2            (5u)
#define PIN_DIP3            (6u)

// Buzzer
#define PIN_BUZZER          (12u)

// Opponent sensors (active LOW)
#define PIN_OPP_L           (11u)
#define PIN_OPP_FL          (20u)
#define PIN_OPP_FC          (21u)
#define PIN_OPP_FR          (22u)
#define PIN_OPP_R           (8u)

// Motor driver (FAULT active LOW)
#define PIN_PWM_L           (15u)
#define PIN_DIR_L           (14u)
#define PIN_PWM_R           (9u)
#define PIN_DIR_R           (10u)
#define PIN_FAULT_L         (19u)
#define PIN_FAULT_R         (25u)

// Servos and IR start module
#define PIN_SERVO1          (16u)
#define PIN_SERVO2          (17u)
#define PIN_IRST            (18u)

// Analog: edge sensors, potentiometer, motor voltage (1:11 divider)
#define PIN_A0              (26u)
#define PIN_A1              (27u)
#define PIN_A2              (28u)
#define PIN_A3              (29u)
#define PIN_EDGE_L          PIN_A0
#define PIN_EDGE_R          PIN_A1
#define PIN_POT             PIN_A2
#define PIN_VOLTAGE_MONITOR PIN_A3

// Serial (GP0/GP1 are free on this board) — verify against the schematic
#define PIN_SERIAL1_TX      (0u)
#define PIN_SERIAL1_RX      (1u)

// Not pinned out
#define PIN_SERIAL2_TX      (31u)
#define PIN_SERIAL2_RX      (31u)

// Not pinned out
#define PIN_SPI0_MISO       (31u)
#define PIN_SPI0_MOSI       (31u)
#define PIN_SPI0_SCK        (31u)
#define PIN_SPI0_SS         (31u)
#define PIN_SPI1_MISO       (31u)
#define PIN_SPI1_MOSI       (31u)
#define PIN_SPI1_SCK        (31u)
#define PIN_SPI1_SS         (31u)

// Wire — shares GP0/GP1 with Serial1; use one or the other
#define PIN_WIRE0_SDA       (0u)
#define PIN_WIRE0_SCL       (1u)
#define PIN_WIRE1_SDA       (31u)
#define PIN_WIRE1_SCL       (31u)

#define SERIAL_HOWMANY      (1u)
#define SPI_HOWMANY         (1u)
#define WIRE_HOWMANY        (1u)

#include "../generic/common.h"
