#pragma once

// MOTOR CONTROL SECTION:
// SimpleFOC Flags
#define SIMPLEFOC_PWM_HIGHSIDE_ACTIVE_HIGH true
#define SIMPLEFOC_PWM_LOWSIDE_ACTIVE_HIGH false

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
// LEDs
#define PIN_LED        (6u)

// Fixed peripheral pin mappings removed, since there aren't
// enough free pins for all peripherals without overlaps.
// Users will need to assign pins as needed per use-case.

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
