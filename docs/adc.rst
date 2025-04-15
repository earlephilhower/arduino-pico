ADC Input Library
=================

The ADC pins can be sampled and recorded by an application using the same
interface as the I2S or PWM Audio libraries.  This allows analog devices which
need to be periodically sampled to be read by applications, easily, such as:

* Analog electret microphones

* Potentiometers

* Light dependent resistors (LDR), etc.


Up to 4 (or 8 in the case of the RP2350B) analog samples can be recorded by the
hardware (``A0`` ... ``A3``), and all recording is done at 16-bit levels (but be
aware that the ADC in the Pico will only ever return values between 0...4095).

The interface for the ``ADCInput`` device is very similar to the ``I2S`` input
device, and most code can be ported simply by instantiating a ``ADCInput``
object in lieu of an ``I2S`` input object and choosing the pins to record.

Since this uses the ADC hardware, no ``analogRead`` or ``analogReadTemp`` calls are
allowed while in use.

ADC Input API
-------------

ADCInput(pin0 [, pin1, pin2, pin3[, pin4, pin5, pin6, pin7])
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Creates an ADC input object which will record the pins specified in the code.
Only pins ``A0`` ... ``A3`` (``A7`` on RP2350B) can be used, and they must be
specified in increasing order (i.e. ``ADCInput(A0, A1);`` is valid,
but ``ADCInput(A1, A0)`` is not.

bool setBuffers(size_t buffers, size_t bufferWords)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Set the number of DMA buffers and their size in 32-bit words.  Call before
``ADCInput::begin()``.

When running at high sample rates, it is recommended to increase the
``bufferWords`` to 32 or higher (i.e. ``adcinput.setBuffers(4, 32);`` ).

bool setPins(pin_size_t pin [, pin1, pin2, pin3])
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Adjusts the pin to record.  Only legal before ``ADCInput::begin()``.

bool setFrequency(long sampleRate)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets the ADC sampling frequency, but does not start recording (however if the
device was already running, it will continue to run at the new frequency).  Note
that every pin requested will be sampled at this frequency, one after the other.
That is, if you have code like this:

.. code:: cpp

    ADCInput adc(A0, A1);
    adc.setFrequency(1000);

``A0`` will be sampled at 0ms, 1ms, 2ms, etc. and ``A1`` will be sampled at 0.5ms
1.5ms, 2.5ms, etc.  Each input is sampled at the proper frequency but offset in time
since there is only one active ADC at a time.

bool begin()/begin(long sampleRate)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Start the ADC input up with the given sample rate, or with the value set
using the prior ``setFrequency`` call.

void end()
~~~~~~~~~~
Stops the ADC Input device.

int read()
~~~~~~~~~~
Reads a single sample of recorded ADC data, as a 16-bit value.  When multiple pins are
recorded the first read will be pin 0, the second will be pin 1, etc.  Applications need
to keep track of which pin is being returned (normally by always reading out all pins
at once).  Will not return until data is available.

int available()
~~~~~~~~~~~~~~~
Returns the number of samples that can be read without potentially blocking.

void onReceive(void (\*fn)(void))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets a callback to be called when a ADC input DMA buffer is fully filled.
Will be in an interrupt context so the specified function must operate
quickly and not use blocking calls like delay().
