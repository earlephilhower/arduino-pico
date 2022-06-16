Analog I/O
==========

Analog Input
------------
For analog inputs, the RP2040 device has a 12-bit, 4-channel ADC +
temperature sensor available on a fixed set of pins (A0...A3).
The standard Arduino calls can be used to read their values (with
3.3V nominally reading as 4095).

int analogRead(pin_size_t pin = A0..A3)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Returns a value from 0...4095 correspionding to the ADC reading
of the specific pin.

void analogReadResolution(int bits)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Determines the resolution (in bits) of the value returned by the analogRead() function.
Default resolution is 10bit.

float analogReadTemp()
~~~~~~~~~~~~~~~~~~~~~~
Returns the temperature, in Celsius, of the onboard thermal sensor.
This reading is not exceedingly accurate and of relatively low
resolution, so it is not a replacement for an external temperature
sensor in many cases.

Analog Outputs
--------------
The RP2040 does not have any onboard DACs, so analog outputs are
simulated using the standard method of using pulse width modulation
(PWM) using the RP20400's hardware PWM units.

While up to 16 PWM channels can be generated, they are not independent
and there are significant restrictions as to allowed pins in parallel.
See the `RP2040 datasheet <https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf>`_ for full details.

Analog Output Restrictions
--------------------------

The PWM generator source clock restricts the legal combinations of
frequency and ranges.  For example, at 1MHz only about 6 bits of range
are possible.  When you define an ``analogWriteFreq`` and ``analogWriteRange``
that can't be fulfilled by the hardware, the frequency will be preserved
but the accuracy (range) will be reduced automatically.  Your code will
still send in the range you specify, but the core itself will transparently
map it into the allowable PWN range.

void analogWriteFreq(uint32_t freq)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets the master PWM frequency used (i.e. how often the PWM output cycles).
From 100Hz to 1MHz are supported.

void analogWriteRange(uint32_t range) and analogWriteResolution(int res)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
These calls set the maximum PWM value (i.e. writing this value will result in
a PWM duty cycle of 100%)/ either explicitly (range) or as a power-of-two
(res).  A range of 16 to 65535 is supported.

void analogWrite(pin_size_t pin, int val)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Writes a PWM value to a specific pin.  The PWM machine is enabled and set to
the requested frequency and scale, and the output is generated.  This will
continue until a ``digitalWrite`` or other digital output is performed.
