#pragma once

// WeAct Studio RP2350B Core Board
// https://github.com/WeActStudio/WeActStudio.RP2350BCoreBoard
//
// RP2350B: 48 GPIO pins (GP0-GP47), ADC auf GP40-GP47 (Kanaele 0-7)
// Pin-Layout: gerade Pins auf der inneren Reihe, ungerade auf der aeusseren
//
// WICHTIG: PICO_RP2350A muss 0 sein, um den B-Die-Zweig (48 GPIOs) in
// ../generic/common.h zu aktivieren. common.h definiert dann automatisch
// D0-D47 sowie A0-A7 (A0=GP40 ... A7=GP47) passend zum WeAct-Pinout -
// eigene Remaps/Nachtraege sind dafuer NICHT noetig.
#define PICO_RP2350A 0 // RP2350B

// Serial
#define PIN_SERIAL1_TX (0u)
#define PIN_SERIAL1_RX (1u)
#define PIN_SERIAL2_TX (8u)
#define PIN_SERIAL2_RX (9u)

// SPI
#define PIN_SPI0_MISO  (16u)
#define PIN_SPI0_MOSI  (19u)
#define PIN_SPI0_SCK   (18u)
#define PIN_SPI0_SS    (17u)
#define PIN_SPI1_MISO  (12u)
#define PIN_SPI1_MOSI  (15u)
#define PIN_SPI1_SCK   (14u)
#define PIN_SPI1_SS    (13u)

// Wire (I2C)
#define PIN_WIRE0_SDA  (4u)
#define PIN_WIRE0_SCL  (5u)
#define PIN_WIRE1_SDA  (26u)
#define PIN_WIRE1_SCL  (27u)

// LED
#define PIN_LED        (25u)

#define SERIAL_HOWMANY (3u)
#define SPI_HOWMANY    (2u)
#define WIRE_HOWMANY   (2u)

// Kein manuelles A0-A7-Remapping noetig: common.h setzt im
// PICO_RP2350A==0-Zweig automatisch A0=GP40 ... A7=GP47, exakt
// passend zum WeAct-ADC-Layout.
#include "../generic/common.h"

// ACHTUNG (Stand des geprueften arduino-pico-Commits 832f2c0):
// PINS_COUNT / NUM_DIGITAL_PINS / NUM_ANALOG_INPUTS werden in
// common.h fest auf 30/30/4 gesetzt - unabhaengig von PICO_RP2350A.
// D30-D47 und A4-A7 sind trotzdem als Konstanten nutzbar (common.h
// definiert sie im B-Die-Zweig), aber falls Code irgendwo ueber
// NUM_DIGITAL_PINS/NUM_ANALOG_INPUTS iteriert, werden GP30-47 dabei
// nicht erfasst. Vor Produktiveinsatz pruefen, ob eine neuere
// arduino-pico-Version diese Werte fuer RP2350B bereits korrekt
// hochsetzt; ggf. hier lokal ueberschreiben:
// #undef PINS_COUNT
// #undef NUM_DIGITAL_PINS
// #undef NUM_ANALOG_INPUTS
// #define PINS_COUNT        (48u)
// #define NUM_DIGITAL_PINS  (48u)
// #define NUM_ANALOG_INPUTS (8u)
// (vor dem #include "../generic/common.h" reicht ein #define nicht,
// da common.h selbst PINS_COUNT setzt - also nach dem Include per
// #undef/#define ueberschreiben und testen, ob Code sich darauf
// verlaesst, bevor ihr das aktiviert.)
