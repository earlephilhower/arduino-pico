// Composite USB example: two CDC serial ports, USB MIDI, and UAC2 audio
// playback/capture added from the sketch, without modifying the core.
// Type \? on the first serial port for commands. See README.md.
//
// Requires Tools->USB Stack->"Pico SDK" (the default), not "Adafruit TinyUSB".
//
// Derived from the USB work in PA3GSB's RP2350-SDR-CW-Interface
// (https://github.com/pa3gsb/RP2350-SDR-CW-Interface) as adapted for
// Arduino-Pico in https://github.com/K7MDL2/K3NG_Keyer_ESP32_BT_Keyboard
// Released to the public domain 2026 by K7MDL2

void composite_begin();
void composite_service();

void setup() {
  Serial.begin(115200);
  composite_begin();
}

void loop() {
  composite_service();
}
