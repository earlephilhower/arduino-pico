Digital I/O
===========

Board-Specific Pins
-------------------
The Raspberry Pi Pico RP2040 chip supports up to 30 digital I/O pins,
however not all boards provide access to all pins.

Input Modes
-----------
The Raspberry Pi Pico has 3 Input modes settings for use with `pinMode`: `INPUT`, `INPUT_PULLUP` and `INPUT_PULLDOWN`

Output Modes (Pad Strength)
---------------------------
The Raspberry Pi Pico has the ability to set the current that a pin (actually the pad associated with it) is capable of supplying. The current can be set to values of 2mA, 4mA, 8mA and 12mA. By default, on a reset, the setting is 4mA. A `pinMode(x, OUTPUT)`, where `x` is the pin number, is also the default setting. 4 settings have been added for use with `pinMode`: `OUTPUT_2MA`, `OUTPUT_4MA`, which has the same behavior as `OUTPUT`, `OUTPUT_8MA` and `OUTPUT_12MA`.

Tone/noTone
-----------
Simple square wave tone generation is possible for up to 8 channels using
Arduino standard ``tone`` calls.  Because these use the PIO to generate the
waveform, they must share resources with other calls such as ``I2S`` or
``Servo`` objects.
