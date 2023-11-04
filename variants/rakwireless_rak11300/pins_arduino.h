#pragma once

// Pin definitions taken from:
//    RAK definition: https://github.com/RAKWireless/RAK-RP-Arduino/
//    RAK datasheet: https://docs.rakwireless.com/Product-Categories/WisDuo/RAK11300-Module/Datasheet/#overview
//    Internal wiring of SX1262 module: https://forum.rakwireless.com/t/rak11300-pinout-rp2040-to-sx1262/8414/

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (5u)
#define PIN_SERIAL2_RX (4u)

// SPI
#define PIN_SPI0_MISO  (16u)
#define PIN_SPI0_MOSI  (19u)
#define PIN_SPI0_SCK   (18u)
#define PIN_SPI0_SS    (17u)

// Hardwired to SX1262 Radio
#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (13u)
// Reset / NReset PIN
#define PIN_SX1262_NRESET   (14u)
// Busy / GPIO PIN
#define PIN_SX1262_BUSY     (15u)
// DIO1 / IRQ PIN
#define PIN_SX1262_DIO1     (29u)
// Antenna Switch power control
#define PIN_SX1262_ANT_PWR  (25u)

// Wire
#define PIN_WIRE0_SDA  (2u)
#define PIN_WIRE0_SCL  (3u)

#define PIN_WIRE1_SDA  (20u)
#define PIN_WIRE1_SCL  (21u)

#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

#include "../generic/common.h"
