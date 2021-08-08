Digital I/O
===========

Board-Specific Pins
-------------------
The Raspberry Pi Pico RP2040 chip supports up to 30 digital I/O pins,
however not all boards provide access to all pins.

Tone/noTone
-----------
Simple square wave tone generation is possible for up to 8 channels using
Arduino standard ``tone`` calls.  Because these use the PIO to generate the
waveform, they must share resources with other calls such as ``I2S`` or
``Servo`` objects.
