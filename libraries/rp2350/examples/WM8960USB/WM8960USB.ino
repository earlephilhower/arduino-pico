// WM8960 USB sound card example for RP2350 (Pico 2 / Pico 2 W): UAC2
// playback/capture through a WM8960 codec over PIO I2S and circular DMA,
// with USB feedback, plus two CDC serial ports and USB MIDI.
// Edit codec.h for wiring. Type \? on the first serial port for commands.
// See README.md.
//
// Requires Tools->USB Stack->"Pico SDK" (the default), not "Adafruit TinyUSB".
//
// Derived from the USB/I2S work in PA3GSB's RP2350-SDR-CW-Interface
// (https://github.com/pa3gsb/RP2350-SDR-CW-Interface) as adapted for
// Arduino-Pico in https://github.com/K7MDL2/K3NG_Keyer_ESP32_BT_Keyboard
// Released to the public domain 2026 by K7MDL2

#include "composite_config.h"

#if !WM8960USB_SUPPORTED

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("This example needs an RP2350 (Pico 2 / Pico 2 W) and a WM8960 codec");
  delay(1000);
}

#else

void composite_begin();
void composite_service();

void setup() {
  Serial.begin(115200);
  composite_begin();
}

void loop() {
  composite_service();
}

#endif
