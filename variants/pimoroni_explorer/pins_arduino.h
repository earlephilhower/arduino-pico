#pragma once

#include <stdint.h>

// Pin definitions taken from:
//    https://github.com/pimoroni/explorer/blob/main/explorer/explorer.h
//    https://github.com/pimoroni/explorer/blob/main/examples/lib/explorer.py
//    https://cdn.shopify.com/s/files/1/0174/1800/files/Pimoroni_Explorer_Schematic.pdf?v=1729171995


//TODO: Is this the chip or board? Do the buttons count?
#define PINS_COUNT          (33u)
#define NUM_DIGITAL_PINS    (33u)
#define NUM_ANALOG_INPUTS   (6u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

// Serial/UART?
#define PIN_SERIAL1_TX      (0u)
#define PIN_SERIAL1_RX      (1u)

//TODO: SPI? Does the QSPI for flash count?
/*
#define PIN_SPI0_MISO       (32u)
#define PIN_SPI0_MOSI       (35u)
#define PIN_SPI0_SCK        (34u)
#define PIN_SPI0_SS         (33u)
*/

#define SERIAL_HOWMANY      (3u)
#define SPI_HOWMANY         (0u)
#define WIRE_HOWMANY        (1u)


/* IO pin definitions */

static const uint8_t GP0 = (0u);
static const uint8_t GP1 = (1u);
static const uint8_t GP2 = (2u);
static const uint8_t GP3 = (3u);
static const uint8_t GP4 = (4u);
static const uint8_t GP5 = (5u);

//TODO: Servos
#define PIN_SERVO1          (6u)
#define PIN_SERVO2          (7u)
#define PIN_SERVO3          (8u)
#define PIN_SERVO4          (9u)
// Pin 10 is NC
// Pin 11 is NC

// on-board speaker
#define PIN_PWM_AUDIO         (12u)
#define PIN_SPEAKER_AMP_ENABLE (13u)

// on-board buttons
//TODO: Should the switches be automatically set to INPUT_PULLUP?
static const uint8_t SWITCH_C = (14u);
static const uint8_t SWITCH_B = (15u);
static const uint8_t SWITCH_A = (16u);
static const uint8_t SWITCH_X = (17u);
static const uint8_t SWITCH_Y = (18u);
static const uint8_t SWITCH_Z = (19u);

// Wire/I2C
#define PIN_WIRE0_SDA       (20u)
#define PIN_WIRE0_SCL       (21u)

// Boot button, also referred to as the user switch
static const uint8_t SWITCH_BOOT = (22u);

// Pin 23 is NC
// Pin 24 is NC
// Pin 25 is NC

//TODO: Define LCD_BL as LED_BUILTIN? it would look awful, but ensure compatibility.
// Using Pin 25 as LED_BUILTIN also works, but the user might wonder why nothing's happening.

//TODO: LCD
#define PIN_LCD_BL          (26u) 
#define PIN_LCD_CS          (27u)
#define PIN_LCD_RS          (28u)
// Pin 29 is NC
#define PIN_LCD_WR          (30u)
#define PIN_LCD_RD          (31u)
#define PIN_LCD_DB0         (32u)
#define PIN_LCD_DB1         (33u)
#define PIN_LCD_DB2         (34u)
#define PIN_LCD_DB3         (35u)
#define PIN_LCD_DB4         (36u)
#define PIN_LCD_DB5         (37u)
#define PIN_LCD_DB6         (38u)
#define PIN_LCD_DB7         (39u)

static const uint8_t ADC0 = (40u);
static const uint8_t ADC1 = (41u);
static const uint8_t ADC2 = (42u);
static const uint8_t ADC3 = (43u);
static const uint8_t ADC4 = (44u);
static const uint8_t ADC5 = (45u);
