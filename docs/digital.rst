Digital I/O
===========

Board-Specific Pins
-------------------
The Raspberry Pi Pico RP2040 chip supports up to 30 digital I/O pins,
however not all boards provide access to all pins.

Pin Notation
------------
When using Analog or Digital I/Os, if you supply an integer it specifies the RP2040 GPIO pin to use. Using Dx or Ax notation (for example, D4 or A3) may be necessary on boards without a direct PCB pin to GPIO mapping.

Input Modes
-----------
The Raspberry Pi Pico has 3 Input modes settings for use with `pinMode`: `INPUT`, `INPUT_PULLUP` and `INPUT_PULLDOWN`

Output Modes (Pad Strength)
---------------------------
The Raspberry Pi Pico has the ability to set the current that a pin (actually the pad associated with it) is capable of supplying. The current can be set to values of 2mA, 4mA, 8mA and 12mA. By default, on a reset, the setting is 4mA. A `pinMode(x, OUTPUT)`, where `x` is the pin number, is also the default setting. 4 settings have been added for use with `pinMode`: `OUTPUT_2MA`, `OUTPUT_4MA`, which has the same behavior as `OUTPUT`, `OUTPUT_8MA` and `OUTPUT_12MA`.

GPIO Output Inversion
---------------------
The RP2040 and RP2350 microcontrollers support configurable output inversion on all GPIO pins.
This feature allows each GPIO to output either the logical value or its inverted equivalent,
without requiring changes to the underlying logic of your application.

This is especially useful when adapting existing libraries or hardware that use opposite logic levels.
By configuring inversion at the GPIO level, you can achieve compatibility without rewriting code.

Function
--------

.. code-block:: c

   void setPinInvert(uint pin, bool invert);

**Parameters:**

- ``pin``: The GPIO number to configure.
- ``invert``:
  
  - ``true`` — Enables output inversion (logical 1 becomes 0, and vice versa).
  - ``false`` — Disables inversion, restoring default non-inverted output.

.. note::

   All GPIO pins are configured as non-inverting outputs by default.

Tone/noTone
-----------
Simple square wave tone generation is possible for up to 8 channels using
Arduino standard ``tone`` calls.  Because these use the PIO to generate the
waveform, they must share resources with other calls such as ``I2S`` or
``Servo`` objects.
