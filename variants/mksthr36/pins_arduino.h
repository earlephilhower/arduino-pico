#pragma once

// Pin definitions taken from:
//    https://datasheets.raspberrypi.org/pico/pico-datasheet.pdf

#define PIN_HE0 (0u)

#define PIN_FAN0 (1u)
#define PIN_FAN1 (2u)
#define PIN_FAN2 (3u)

#define PIN_E0_DIR (4u)
#define PIN_E0_STEP (5u)

#define PIN_SERIAL1_TX (6u)
#define PIN_E0_UART (6u)
#define PIN_SERIAL1_RX (31u)
#define PIN_E0_DIAG (7u)
#define PIN_E0_EN (10u)

#define PIN_CAN_RX (8u)
#define PIN_CAN_TX (9u)
#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

#define PIN_3D_TOUCH (11u)

//SPI
#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (15u)
#define PIN_SPI1_SCK   (14u)
#define PIN_SPI1_SS    (13u)

#define PIN_SPI0_MISO  (16u)
#define PIN_SPI0_MOSI  (19u)
#define PIN_SPI0_SCK   (18u)
#define PIN_SPI0_SS    (17u)

#define PIN_NEOPIXEL   (20u)
#define PIN_ZPLUS      (21u)

// Wire
#define PIN_WIRE0_SDA  (22u)
#define PIN_WIRE0_SCL  (23u)
#define PIN_I2C_SDA  (22u)
#define PIN_I2C_SCL  (23u)

#define PIN_IO24     (24u)
#define PIN_IO25     (25u)

#define PIN_TH0      (26u)
#define PIN_IPO29    (29u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (1u)

#include "../generic/common.h"
