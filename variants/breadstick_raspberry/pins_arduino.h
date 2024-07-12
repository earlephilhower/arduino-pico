#pragma once

#include <stdint.h>


// Pin definitions taken from:
//    https://github.com/Breadstick-Innovations/Raspberry-Breadstick


// Serial
#define PIN_SERIAL1_TX (20u)
#define PIN_SERIAL1_RX (21u)

// Not pinned
#define PIN_SERIAL2_TX (31)
#define PIN_SERIAL2_RX (31)

// SPI

#define PIN_SPI1_MISO  (8u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (9u)

// Wire
#define PIN_WIRE0_SDA  (12u)
#define PIN_WIRE0_SCL  (13u)

#define PIN_WIRE1_SDA  (22u)
#define PIN_WIRE1_SCL  (23u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (2u)

#define PINS_COUNT          (22u)
#define NUM_DIGITAL_PINS    (22u)
#define NUM_ANALOG_INPUTS   (4u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)
#define WIRE_INTERFACES_COUNT (WIRE_HOWMANY)

// TODO - fix to use newly defined common.h
static const uint8_t D1 = (27u);
static const uint8_t D2 = (26u);
static const uint8_t D3 = (11u);
static const uint8_t D4 = (10u);
static const uint8_t D5 = (9u);
static const uint8_t D6 = (8u);
static const uint8_t D7 = (7u);
static const uint8_t D8 = (6u);
static const uint8_t D9 = (5u);
static const uint8_t D10 = (24u);
static const uint8_t D11 = (23u);
static const uint8_t D12 = (22u);
static const uint8_t D13 = (21u);
static const uint8_t D14 = (20u);
static const uint8_t D15 = (19u);
static const uint8_t D16 = (18u);
static const uint8_t D17 = (29u);
static const uint8_t D18 = (28u);


#ifdef __PIN_A2
static const uint8_t A0 = __PIN_A0;
#else
static const uint8_t A0 = (26u);
#endif

#ifdef __PIN_A1
static const uint8_t A1 = __PIN_A1;
#else
static const uint8_t A1 = (27u);
#endif

#ifdef __PIN_A18
static const uint8_t A2 = __PIN_A2;
#else
static const uint8_t A2 = (28u);
#endif

#ifdef __PIN_A17
static const uint8_t A3 = __PIN_A3;
#else
static const uint8_t A3 = (29u);
#endif

static const uint8_t SS = PIN_SPI1_SS;
static const uint8_t MOSI = PIN_SPI1_MOSI;
static const uint8_t MISO = PIN_SPI1_MISO;
static const uint8_t SCK = PIN_SPI1_SCK;

static const uint8_t SDA = PIN_WIRE1_SDA;
static const uint8_t SCL = PIN_WIRE1_SCL;

static const uint8_t IMU_SDA = PIN_WIRE0_SDA;
static const uint8_t IMU_SCL = PIN_WIRE0_SCL;

static const uint8_t DOTSTAR_CLOCK = (16u);
static const uint8_t DOTSTAR_DATA = (17u);
