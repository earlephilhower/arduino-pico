#pragma once


// LEDs
#define PIN_LED        (24u)

#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

// TMC UART
#define PIN_SERIAL2_TX (4u)
#define PIN_SERIAL2_RX (5u)


// SPI
#define PIN_SPI1_MISO  (31u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (31u)
#define PIN_SPI1_SS    (31u) // not pinned out

// Not pinned out
#define PIN_SPI0_MISO  (31u)
#define PIN_SPI0_MOSI  (31u)
#define PIN_SPI0_SCK   (31u)
#define PIN_SPI0_SS    (31u)

// Wire
#define PIN_WIRE0_SDA  (31u)
#define PIN_WIRE0_SCL  (31u)

// Not pinned out
#define PIN_WIRE1_SDA  (31u)
#define PIN_WIRE1_SCL  (31u)


#define SERIAL_HOWMANY (2u)
#define SPI_HOWMANY    (0u)
#define WIRE_HOWMANY   (0u)

#include "../generic/common.h"
