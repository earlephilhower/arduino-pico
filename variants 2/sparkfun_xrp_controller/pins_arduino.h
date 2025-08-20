#pragma once

#define PICO_RP2350A 0 // RP2350B

#include <cyw43_wrappers.h>

// XRP default pin names
#define MOTOR_L_IN_1 (34u)
#define MOTOR_L_IN_2 (35u)
#define MOTOR_R_IN_1 (32u)
#define MOTOR_R_IN_2 (33u)
#define MOTOR_3_IN_1 (20u)
#define MOTOR_3_IN_2 (21u)
#define MOTOR_4_IN_1 (10u)
#define MOTOR_4_IN_2 (11u)
#define MOTOR_L_ENCODER_A (30u)
#define MOTOR_L_ENCODER_B (31u)
#define MOTOR_R_ENCODER_A (24u)
#define MOTOR_R_ENCODER_B (25u)
#define MOTOR_3_ENCODER_A (22u)
#define MOTOR_3_ENCODER_B (23u)
#define MOTOR_4_ENCODER_A (2u)
#define MOTOR_4_ENCODER_B (3u)
#define MOTOR_L_CURRENT (40u)
#define MOTOR_R_CURRENT (43u)
#define MOTOR_3_CURRENT (41u)
#define MOTOR_4_CURRENT (42u)
#define SERVO_1 (6u)
#define SERVO_2 (9u)
#define SERVO_3 (7u)
#define SERVO_4 (8u)
#define I2C_SDA_0 (4u)
#define I2C_SCL_0 (5u)
#define I2C_SDA_1 (38u)
#define I2C_SCL_1 (39u)
#define DISTANCE_TRIGGER (0u)
#define DISTANCE_ECHO (1u)
#define LINE_L (44u)
#define LINE_R (45u)
#define BOARD_VIN_MEASURE (46u)
#define BOARD_USER_BUTTON (36u)
#define BOARD_NEOPIXEL (37u)
#define BOARD_LED (PIN_LED)

// XRP alternate pin names
#define ML_IN_1 (34u)
#define ML_IN_2 (35u)
#define MR_IN_1 (32u)
#define MR_IN_2 (33u)
#define M3_IN_1 (20u)
#define M3_IN_2 (21u)
#define M4_IN_1 (10u)
#define M4_IN_2 (11u)
#define ML_ENC_A (30u)
#define ML_ENC_B (31u)
#define MR_ENC_A (24u)
#define MR_ENC_B (25u)
#define M3_ENC_A (22u)
#define M3_ENC_B (23u)
#define M4_ENC_A (2u)
#define M4_ENC_B (3u)
#define ML_CUR (40u)
#define MR_CUR (43u)
#define M3_CUR (41u)
#define M4_CUR (42u)
#define S1 (6u)
#define S2 (9u)
#define S3 (7u)
#define S4 (8u)
#define SDA_0 (4u)
#define SCL_0 (5u)
#define SDA_1 (38u)
#define SCL_1 (39u)
#define RANGE_TRIGGER (0u)
#define RANGE_ECHO (1u)
#define REFLECTANCE_L (44u)
#define REFLECTANCE_R (45u)
#define BRD_VIN (46u)
#define BRD_USR_BTN (36u)
#define BRD_RGB_LED (37u)
#define BRD_LED (PIN_LED)

// LEDs
#define PIN_LED        (64u)

#define PIN_NEOPIXEL   (37)
#define NUM_NEOPIXEL   (1)

// Serial
#define PIN_SERIAL1_TX (12u)
#define PIN_SERIAL1_RX (13u)

#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
#define PIN_SPI0_MISO  (16u)
#define PIN_SPI0_MOSI  (19u)
#define PIN_SPI0_SCK   (18u)
#define PIN_SPI0_SS    (17u)

#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (15u)
#define PIN_SPI1_SCK   (14u)
#define PIN_SPI1_SS    (13u)

// Wire
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)

#define PIN_WIRE1_SDA  (38u)
#define PIN_WIRE1_SCL  (39u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

// PSRAM
#define RP2350_PSRAM_CS         (47u)
#define RP2350_PSRAM_MAX_SCK_HZ (109*1000*1000)

#include "../generic/common.h"
