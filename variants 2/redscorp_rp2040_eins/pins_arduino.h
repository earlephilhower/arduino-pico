/*  RP2040-Eins board support
    URL: https://github.com/red-scorp/RP2040-Eins
    Based on Arduino Nano Connect configuration */

#pragma once

// Pin definitions taken from:
//    https://datasheets.raspberrypi.org/pico/pico-datasheet.pdf
static const uint8_t D0 = (1u);
static const uint8_t D1 = (0u);
static const uint8_t D2 = (25u);
static const uint8_t D3 = (15u);
static const uint8_t D4 = (16u);
static const uint8_t D5 = (17u);
static const uint8_t D6 = (18u);
static const uint8_t D7 = (19u);
static const uint8_t D8 = (20u);
static const uint8_t D9 = (21u);
static const uint8_t D10 = (5u);
static const uint8_t D11 = (7u);
static const uint8_t D12 = (4u);
static const uint8_t D13 = (6u);
static const uint8_t D14 = (26u);
static const uint8_t D15 = (27u);
static const uint8_t D16 = (28u);
static const uint8_t D17 = (29u);
static const uint8_t D18 = (12u);
static const uint8_t D19 = (13u);
static const uint8_t D20 = (22u);
static const uint8_t D21 = (23u);
static const uint8_t D22 = (9u);
static const uint8_t D23 = (10u);
static const uint8_t D24 = (11u);
static const uint8_t D25 = (8u);
static const uint8_t D26 = (2u);
static const uint8_t D27 = (24u);
static const uint8_t D28 = (3u);
static const uint8_t D29 = (14u);

static const uint8_t A0 = (26u);
static const uint8_t A1 = (27u);
static const uint8_t A2 = (28u);
static const uint8_t A3 = (29u);

// LEDs
#define PIN_LED        (D13)
#define PIN_LED_R      (D22)
#define PIN_LED_G      (D23)
#define PIN_LED_B      (D24)
#define LED_BUILTIN    PIN_LED

// Serial
#define PIN_SERIAL1_TX (D1)
#define PIN_SERIAL1_RX (D0)

#define PIN_SERIAL2_TX  (D25)
#define PIN_SERIAL2_RX  (D26)
#define PIN_SERIAL2_CTS (D27)
#define PIN_SERIAL2_RTS (D28)

// SPI
#define PIN_SPI0_MISO  (D12)
#define PIN_SPI0_MOSI  (D11)
#define PIN_SPI0_SCK   (D13)
#define PIN_SPI0_SS    (D10)

#define PIN_SPI1_MISO  (D25)
#define PIN_SPI1_MOSI  (D28)
#define PIN_SPI1_SCK   (D29)
#define PIN_SPI1_SS    (D10)

// Wire
#define PIN_WIRE0_SDA  (D18)
#define PIN_WIRE0_SCL  (D19)

#define PIN_WIRE1_SDA  (D14)
#define PIN_WIRE1_SCL  (D15)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (1u)

#define PINS_COUNT          (30u)
#define NUM_DIGITAL_PINS    (30u)
#define NUM_ANALOG_INPUTS   (4u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

#define DigitalPinToPinName(p) (p)

static const uint8_t SS = PIN_SPI0_SS;
static const uint8_t MOSI = PIN_SPI0_MOSI;
static const uint8_t MISO = PIN_SPI0_MISO;
static const uint8_t SCK = PIN_SPI0_SCK;

// Some random stuff

#define SERIAL_PORT_USBVIRTUAL      SerialUSB
#define SERIAL_PORT_MONITOR         SerialUSB
#define SERIAL_PORT_HARDWARE        Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial2

#define CRYPTO_WIRE		Wire

#define USB_MAX_POWER	(500)
