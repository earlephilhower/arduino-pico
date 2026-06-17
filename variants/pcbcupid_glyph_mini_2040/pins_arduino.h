/*
 * pins_arduino.h - PCBCupid GLYPH Mini 2040
 * RP2040, W25Q64 8MB external flash
 *
 * Available GPIOs: GP0–GP15, GP26–GP29
 * Not broken out: GP16 (LED, internal), GP17, GP18, GP19, GP20, GP21, GP22, GP23, GP24, GP25
 *
 * Pin overlap notice:
 *   GP4/GP5  — shared between SPI0 (MISO/SS) and UART1 (TX/RX) and I2C0 (SDA/SCL)
 *   GP8/GP9  — shared between SPI1 (RX/CSn) and UART1 alt (TX/RX) and I2C0 (SDA/SCL)
 *   Use either UART1 or I2C0 on GP4/GP5 or GP8/GP9 — not both simultaneously.
 */

#pragma once

// LED
#define PIN_LED        (16u)

// Serial0 (UART0) — GP0/GP1
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)

// Serial1 (UART1) — GP4/GP5
// NOTE: GP4/GP5 overlap with I2C0 SDA/SCL and SPI0 MISO/SS
// Use UART1 OR I2C0 on these pins, not both
#define PIN_SERIAL2_TX (4u)
#define PIN_SERIAL2_RX (5u)

// SPI0 — GP2/GP3/GP4/GP5
// NOTE: GP4(MISO) and GP5(SS) overlap with UART1 TX/RX and I2C0 SDA/SCL
#define PIN_SPI0_MISO  (4u)
#define PIN_SPI0_MOSI  (3u)
#define PIN_SPI0_SCK   (2u)
#define PIN_SPI0_SS    (5u)

// SPI1 — GP10/GP11/GP12/GP13
#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (11u)
#define PIN_SPI1_SCK   (10u)
#define PIN_SPI1_SS    (13u)

// Wire0 (I2C0) — GP8/GP9
// NOTE: GP8/GP9 overlap with UART1 alternate TX/RX
// Use I2C0 OR UART1 alt on these pins, not both
#define PIN_WIRE0_SDA  (8u)
#define PIN_WIRE0_SCL  (9u)

// Wire1 (I2C1) — GP26/GP27
#define PIN_WIRE1_SDA  (26u)
#define PIN_WIRE1_SCL  (27u)

// ADC — GP26/GP27/GP28/GP29
#define PIN_A0         (26u)
#define PIN_A1         (27u)
#define PIN_A2         (28u)
#define PIN_A3         (29u)

#define PINS_COUNT          (30u)
#define NUM_DIGITAL_PINS    (30u)
#define NUM_ANALOG_INPUTS   (4u)
#define NUM_ANALOG_OUTPUTS  (0u)

#define ADC_RESOLUTION      (12u)

#define SERIAL_HOWMANY      (3u)
#define SPI_HOWMANY         (2u)
#define WIRE_HOWMANY        (2u)

// No NEOPIXEL on this board

#include "../generic/common.h"
