#pragma once

// MOTOR CONTROL SECTION:
// SimpleFOC Flags
#define SIMPLEFOC_PWM_HIGHSIDE_ACTIVE_HIGH true
#define SIMPLEFOC_PWM_LOWSIDE_ACTIVE_HIGH false

// Important Constants
#define CURR_SENSE_RES  (0.001f)
#define CURR_SENSE_GAIN (66.0f)

// Power Stage Control Pins
#define PIN_PWMH_A     (0u)
#define PIN_PWML_A     (1u)
#define PIN_PWMH_B     (2u)
#define PIN_PWML_B     (3u)
#define PIN_PWMH_C     (4u)
#define PIN_PWML_C     (5u)

// Hall-Effect Angle Sensor Pins
#define PIN_HALL_RX    (16u)
#define PIN_HALL_CS    (17u)
#define PIN_HALL_SCK   (18u)

// Current Sensing Pins
#define PIN_IOUT_A     (26u)    // Analog Input
#define PIN_IOUT_B     (27u)    // Analog Input

// Power Supply Feedback Pins
#define PIN_VBUS_DET   (7u)     // Digital Input
#define PIN_VCC_SENSE  (28u)    // Analog Input


// STANDARD SECTION:
// LED
#define PIN_LED        (6u)

// Serial
#define PIN_SERIAL1_TX (12u)
#define PIN_SERIAL1_RX (13u)

#define PIN_SERIAL2_TX (20u)
#define PIN_SERIAL2_RX (21u)

// NOTE: SPI0 is used by the on-board magnetic angle sensor,
// do not change the pins assigned to this SPI object!
// SPI
#define PIN_SPI0_MISO  (16u)
#define PIN_SPI0_MOSI  (19u)
#define PIN_SPI0_SCK   (18u)
#define PIN_SPI0_SS    (17u)

#define PIN_SPI1_MISO  (8u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (9u)

// Wire
#define PIN_WIRE0_SDA  (24u)
#define PIN_WIRE0_SCL  (25u)

#define PIN_WIRE1_SDA  (22u)
#define PIN_WIRE1_SCL  (23u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
