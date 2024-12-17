// GroundStudio Marble Pico
//
// Reference Pinout:
//	https://raw.githubusercontent.com/GroundStudio/GroundStudio_Marble_Pico/main/Documentation/REV0.0.3/RLJDMV_GS%20REV0.0.3%20GroundStudio%20Marble%20Pico%20Pinout%20REV%201.1.pdf
// Reference Schematic:
//	https://raw.githubusercontent.com/GroundStudio/GroundStudio_Marble_Pico/main/Documentation/REV0.0.3/RLJDMV_GS%20REV0.0.3%20GroundStudio%20Marble%20Pico%20Schematic.pdf

// Built-in LED
#define PIN_LED	(25u)

// Built-in battery charging circuit
#define PIN_VBAT	(24u) // J17 Default Connection

// Built-in SD reader
#define PIN_SD_SWITCH	(24u) // J17 Alternative Connection
#define PIN_SPI0_MISO	(16u)
#define PIN_SPI0_MOSI	(19u)
#define PIN_SPI0_SCK	(18u)
#define PIN_SPI0_SS	(17u)

// ADC GPIO pins
#define PIN_A0	(26u)
#define PIN_A1	(27u)
#define PIN_A2	(28u)

// Serial
#define PIN_SERIAL1_TX	(0u)
#define PIN_SERIAL1_RX	(1u)

#define PIN_SERIAL2_TX	(8u)
#define PIN_SERIAL2_RX	(9u)

// SPI
#define PIN_SPI1_MISO	(12u)
#define PIN_SPI1_MOSI	(11u)
#define PIN_SPI1_SCK	(10u)
#define PIN_SPI1_SS	(13u)

// Wire
// Built-in I2C connector
#define PIN_WIRE0_SDA	(4u)
#define PIN_WIRE0_SCL	(5u)

#define PIN_WIRE1_SDA	(26u)
#define PIN_WIRE1_SCL	(27u)

#define SERIAL_HOWMANY	(3u)
#define SPI_HOWMANY	(2u)
#define WIRE_HOWMANY	(2u)

#include "../generic/common.h"
