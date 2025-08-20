#pragma once


// LEDs
#define PIN_LED        (13u)

// Extra hardware!
#define PIN_SWITCH            0
#define PIN_SPEAKER_ENABLE   14
#define PIN_SPEAKER          16
#define PIN_ROTB             17
#define PIN_ROTA             18
#define OLED_CS              22
#define OLED_RST             23
#define OLED_DC              24

#define PIN_NEOPIXEL         19
#define NUM_NEOPIXEL         12

// Not pinned out
#define PIN_SERIAL1_TX (31u)
#define PIN_SERIAL1_RX (31u)

// Not pinned out
#define PIN_SERIAL2_TX (31u)
#define PIN_SERIAL2_RX (31u)

// SPI
#define PIN_SPI1_MISO  (28u)
#define PIN_SPI1_MOSI  (27u)
#define PIN_SPI1_SCK   (26u)
#define PIN_SPI1_SS    (31u) // not pinned out

// Not pinned out
#define PIN_SPI0_MISO  (31u)
#define PIN_SPI0_MOSI  (31u)
#define PIN_SPI0_SCK   (31u)
#define PIN_SPI0_SS    (31u)

// Wire
#define PIN_WIRE0_SDA  (20u)
#define PIN_WIRE0_SCL  (21u)

// Not pinned out
#define PIN_WIRE1_SDA  (31u)
#define PIN_WIRE1_SCL  (31u)

#define SERIAL_HOWMANY (1u)
#define SPI_HOWMANY    (1u)
#define WIRE_HOWMANY   (1u)

#include "../generic/common.h"
