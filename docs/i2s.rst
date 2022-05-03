I2S (Digital Audio) Audio Library
=================================

While the RP2040 chip on the Raspberry Pi Pico does not include a hardware
I2S device, it is possible to use the PIO (Programmable I/O) state machines
to implement one dynamically.

Digital audio input and output are supported at 8, 16, 24, and 32 bits per
sample.

Theoretically up to 6 I2S ports may be created, but in practice there
may not be enough resources (DMA, PIO SM) to actually create and use so
many.

Create an I2S port by instantiating a variable of the I2S class
specifying the direction.  Configure it using API calls below before
using it.


I2S Class API
-------------

I2S(OUTPUT)
~~~~~~~~~~~
Creates an I2S output port.  Needs to be connected up to the
desired pins (see below) and started before any output can happen.


I2S(INPUT)
~~~~~~~~~~
Creates an I2S input port.  Needs to be connected up to the
desired pins (see below) and started before any output can happen.

bool setBCLK(pin_size_t pin)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets the BCLK pin of the I2S device.  The LRCLK/word clock will be ``pin + 1``
due to limitations of the PIO state machines.  Call this before ``I2S::begin()``

bool setDATA(pin_size_t pin)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets the DOUT or DIN pin of the I2S device.  Any pin may be used.
Call before ``I2S::begin()``

bool setBitsPerSample(int bits)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Specify how many bits per audio sample to read or write.  Note that
for 24-bit samples, audio samples must be left-aligned (i.e. bits 31...8).
Call before ``I2S::begin()``

bool setFrequency(ong sampleRate)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets the word clock frequency, but does not start the I2S device if not
already running.  May be called after ``I2S::begin()`` to change the
sample rate on-the-fly.

bool begin(long sampleRate)
~~~~~~~~~~~~~~~~~~~~~~~~~~~
Start the I2S device up with the given sample rate.  Can be called without
parameters if ``I2S::setFrequency`` has already been set.

void end()
~~~~~~~~~~
Stops the I2S device.

void flush()
~~~~~~~~~~~~
Not implemented

size_t write(uint8_t)
~~~~~~~~~~~~~~~~~~~~~
Provided for compatibility, but not very useful.  Writes a sample from 0...255
to the I2S buffer.  See ``write(int16_t)`` for a better one

size_t write(const uint8_t \*buffer, size_t size)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Transfers number of bytes from an application buffer to the I2S output buffer.
Be aware that ``size`` is in *bytes** and not samples.  Size must be a multiple
of **4 bytes** (i.e. one left/right sample).  Will not block, so check
the return value to find out how many bytes were actually written.

int availableForWrite()
~~~~~~~~~~~~~~~~~~~~~~~
Returns the number of **32-bit L/R samples** that can be written without
potentially blocking.

bool setFrequency(int newFreq)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Adjusts the I2S output frequency.  When changing frequency while I2S output
is underway, be sure to use ``I2S.flush()`` before changing the frequency to
ensure the older data is played at the right frequency.

size_t write(int16_t)
~~~~~~~~~~~~~~~~~~~~~
Writes a single 16-bit left or right sample to the I2S output buffer.  The
user is required to ensure that an even number of samples is written (i.e.
left and right) over the application lifetime.  The application also needs
to track which sample is next to be written (right/left).  For this reason,
the ``write(void *b, size_t lrsamples)`` call may be easier and faster to use.

size_t write(const void \*buffer, size_t lrsamples)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Writes up to size left+right packed samples to the I2S device.  Non-blocking,
will writefrom 0...size samples and return that count.  Be sure your app
handles partial writes (i.e. by ``yield()`` ing and then retrying to write the
remaining data.)

The ``onTransmit`` callback is not supported.
