#pragma once

#include <stdint.h>

// Pin definitions taken from:
//    https://github.com/pimoroni/explorer/blob/main/explorer/
//    https://github.com/pimoroni/explorer/blob/main/examples/lib/explorer.py
//    https://cdn.shopify.com/s/files/1/0174/1800/files/Pimoroni_Explorer_Schematic.pdf?v=1729171995

#define PICO_RP2350A 0

//TODO: Do the buttons count?
#define NUM_DIGITAL_PINS    (12u)
#define NUM_ANALOG_INPUTS   (6u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

#define SERIAL_HOWMANY      (0u)
#define SPI_HOWMANY         (0u)
#define WIRE_HOWMANY        (1u)


/* IO pin definitions */

static const uint8_t GP0 = (0u);
static const uint8_t GP1 = (1u);
static const uint8_t GP2 = (2u);
static const uint8_t GP3 = (3u);
static const uint8_t GP4 = (4u);
static const uint8_t GP5 = (5u);

//Servos
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
//TODO: Call it SWITCH_BOOT or SWITCH_USER?
static const uint8_t SWITCH_BOOT = (22u);

// Pin 23 is NC
// Pin 24 is NC
// Pin 25 is NC
#define PIN_LED 25 // The LED will not blink, but sketches will compile

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

#define LED_BUILTIN PIN_LED

// Definitions to make people happy (these don't actually exist) 
// I'm setting them to GPIO10, which is NC.

// Serial
#define PIN_SERIAL1_TX (10u)
#define PIN_SERIAL1_RX (10u)

#define PIN_SERIAL2_TX (10u)
#define PIN_SERIAL2_RX (10u)

// SPI
#define PIN_SPI0_MISO  (10u)
#define PIN_SPI0_MOSI  (10u)
#define PIN_SPI0_SCK   (10u)
#define PIN_SPI0_SS    (10u)

#define PIN_SPI1_MISO  (10u)
#define PIN_SPI1_MOSI  (10u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (10u)

// Wire1
#define PIN_WIRE1_SDA  (10u)
#define PIN_WIRE1_SCL  (10u)
