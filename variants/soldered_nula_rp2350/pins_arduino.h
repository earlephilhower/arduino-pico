
#include <stdint.h>
#include <cyw43_wrappers.h>

#define PICO_RP2350A 0 // RP2350B

#define PINS_COUNT          (48u)
#define NUM_DIGITAL_PINS    (48u)
#define NUM_ANALOG_INPUTS   (7u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)

//LEDs
#define PIN_NEOPIXEL   (26u)
#define NUM_NEOPIXEL   (1u)

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

#define PIN_SERIAL2_TX (41u)
#define PIN_SERIAL2_RX (42u)

// SPI0
#define PIN_SPI0_MISO  (2u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (4u)
#define PIN_SPI0_SS    (5u)

#define PIN_SPI0_POCI  PIN_SPI0_MISO
#define PIN_SPI0_PICO  PIN_SPI0_MOSI
#define PIN_SPI0_CS    PIN_SPI0_SS

// SD Card SPI
#define PIN_SPI1_MISO  (30u)
#define PIN_SPI1_MOSI  (31u)
#define PIN_SPI1_SCK   (32u)
#define PIN_SPI1_SS    (33u) // CS pin for SD card.

#define PIN_SPI1_POCI  PIN_SPI1_MISO
#define PIN_SPI1_PICO  PIN_SPI1_MOSI
#define PIN_SPI1_CS    PIN_SPI1_SS

// Wire
#define PIN_WIRE0_SDA  (8u)
#define PIN_WIRE0_SCL  (9u)

#define PIN_WIRE1_SDA  (30u)
#define PIN_WIRE1_SCL  (31u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)
#define WIRE_INTERFACES_COUNT (WIRE_HOWMANY)

/* Pin mappings for marked pins on the board */
// UART0
static const uint8_t D0 = (0u);
static const uint8_t D1 = (1u);

// I2C0
static const uint8_t D4 = (4u);
static const uint8_t D5 = (5u);

// GPIO
static const uint8_t D6 = (6u);
static const uint8_t D7 = (7u);

// SD Card/SPI1
static const uint8_t D8 = (8u);
static const uint8_t D9 = (9u);
static const uint8_t D10 = (10u);
static const uint8_t D11 = (11u);


static const uint8_t D31 = (31u);
static const uint8_t D32 = (32u);
static const uint8_t D33 = (33u);
static const uint8_t D34 = (34u);
static const uint8_t D35 = (35u);

// Analog Inputs are also digital capable.
static const uint8_t D40 = (40u);
static const uint8_t D41 = (41u);
static const uint8_t D42 = (42u);
static const uint8_t D43 = (43u);
static const uint8_t D44 = (44u);
static const uint8_t D45 = (45u);
static const uint8_t D46 = (46u);
static const uint8_t D47 = (47u);

static const uint8_t A0 = (40u);
static const uint8_t A1 = (41u);
static const uint8_t A2 = (42u);
static const uint8_t A3 = (43u);
static const uint8_t A4 = (44u);
static const uint8_t A5 = (45u);
static const uint8_t A6 = (46u);
static const uint8_t A7 = (47u);

// SD Card detect - Active Low
static const uint8_t SD_ENABLE = (4u);

// Low battery alert - Active Low
static const uint8_t BATT_ALRT_N = (6u);

// Power enable for peripherals - qwiic, sd, rgb. Default HIGH via HW jumper.
static const uint8_t PERIPHERAL_POWER_ENABLE = (7u);

// WiFi power GPIO
static const uint8_t WRL_ON = (23u);

// User button
static const uint8_t USER_BTN = (27u);


static const uint8_t SS = PIN_SPI0_SS;
static const uint8_t CS = PIN_SPI0_CS;

static const uint8_t MOSI = PIN_SPI0_MOSI;
static const uint8_t PICO = PIN_SPI0_PICO;

static const uint8_t MISO = PIN_SPI0_MISO;
static const uint8_t POCI = PIN_SPI0_POCI;

static const uint8_t SCK = PIN_SPI0_SCK;

static const uint8_t SDA = PIN_WIRE0_SDA;
static const uint8_t SCL = PIN_WIRE0_SCL;

static const uint8_t SDA1 = PIN_WIRE1_SDA;
static const uint8_t SCL1 = PIN_WIRE1_SCL;

static const uint8_t RX = PIN_SERIAL1_RX;
static const uint8_t TX = PIN_SERIAL1_TX;
