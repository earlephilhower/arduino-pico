#pragma once

#include <cyw43_wrappers.h>

// XRP default pin names
#define MOTOR_L_IN_1 (6u)
#define MOTOR_L_IN_2 (7u)
#define MOTOR_R_IN_1 (14u)
#define MOTOR_R_IN_2 (15u)
#define MOTOR_3_IN_1 (2u)
#define MOTOR_3_IN_2 (3u)
#define MOTOR_4_IN_1 (10u)
#define MOTOR_4_IN_2 (11u)
#define MOTOR_L_ENCODER_A (4u)
#define MOTOR_L_ENCODER_B (5u)
#define MOTOR_R_ENCODER_A (12u)
#define MOTOR_R_ENCODER_B (13u)
#define MOTOR_3_ENCODER_A (0u)
#define MOTOR_3_ENCODER_B (1u)
#define MOTOR_4_ENCODER_A (8u)
#define MOTOR_4_ENCODER_B (9u)
#define SERVO_1 (16u)
#define SERVO_2 (17u)
#define I2C_SDA_1 (18u)
#define I2C_SCL_1 (19u)
#define DISTANCE_TRIGGER (20u)
#define DISTANCE_ECHO (21u)
#define LINE_L (26u)
#define LINE_R (27u)
#define BOARD_VIN_MEASURE (28u)
#define BOARD_USER_BUTTON (22u)
#define BOARD_LED (PIN_LED)

// XRP alternate pin names
#define ML_IN_1 (6u)
#define ML_IN_2 (7u)
#define MR_IN_1 (14u)
#define MR_IN_2 (15u)
#define M3_IN_1 (2u)
#define M3_IN_2 (3u)
#define M4_IN_1 (10u)
#define M4_IN_2 (11u)
#define ML_ENC_A (4u)
#define ML_ENC_B (5u)
#define MR_ENC_A (12u)
#define MR_ENC_B (13u)
#define M3_ENC_A (0u)
#define M3_ENC_B (1u)
#define M4_ENC_A (8u)
#define M4_ENC_B (9u)
#define S1 (16u)
#define S2 (17u)
#define SDA_1 (18u)
#define SCL_1 (19u)
#define RANGE_TRIGGER (20u)
#define RANGE_ECHO (21u)
#define REFLECTANCE_L (26u)
#define REFLECTANCE_R (27u)
#define BRD_VIN (28u)
#define BRD_USR_BTN (22u)
#define BRD_LED (PIN_LED)

// LEDs
#define PIN_LED        (64u)

// Serial
#define PIN_SERIAL1_TX (31u)
#define PIN_SERIAL1_RX (31u)

#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI0_MISO  (31u)
#define PIN_SPI0_MOSI  (31u)
#define PIN_SPI0_SCK   (31u)
#define PIN_SPI0_SS    (31u)

#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u)

// Wire
#define PIN_WIRE0_SDA  (18u)
#define PIN_WIRE0_SCL  (19u)

#define PIN_WIRE1_SDA  (31u)
#define PIN_WIRE1_SCL  (31u)

// XRP Beta Controller uses I2C1 for Qwiic connector, make that the default
#ifndef __WIRE0_DEVICE
#define __WIRE0_DEVICE i2c1
#endif
#ifndef __WIRE1_DEVICE
#define __WIRE1_DEVICE i2c0
#endif

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
