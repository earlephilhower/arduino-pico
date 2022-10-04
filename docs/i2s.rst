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
desired pins (see below) and started before any input can happen.

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

bool setBuffers(size_t buffers, size_t bufferWords, int32_t silenceSample = 0)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Set the number of DMA buffers and their size in 32-bit words as well as
the word to fill when no data is available to send to the I2S hardware.
Call before ``I2S::begin()``.

bool setFrequency(long sampleRate)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets the word clock frequency, but does not start the I2S device if not
already running.  May be called after ``I2S::begin()`` to change the
sample rate on-the-fly.

bool begin()/begin(long sampleRate)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Start the I2S device up with the given sample rate, or with the value set
using the prior ``setFrequency`` call.

void end()
~~~~~~~~~~
Stops the I2S device.

void flush()
~~~~~~~~~~~~
Waits until all the I2S buffers have been output.

size_t write(uint8_t/int8_t/int16_t/int32_t)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Writes a single sample of ``bitsPerSample`` to the buffer.  It is up to the
user to keep track of left/right channels.   Note this writes data equivalent
to one channel's data, not the size of the passed in variable (i.e. if you have
a 16-bit sample size and ``write((int8_t)-5); write((int8_t)5);`` you will have
written **2 samples** to the I2S buffer of whatever the I2S size, not a single
16-bit sample.

This call will block (wait) until space is available to actually write
the data.

size_t write(int32_t val, bool sync)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Writes 32 bits of data to the I2S buffer (regardless of the configured I2S
bit size).  When ``sync`` is true, it will not return until the data has
been writte.  When ``sync`` is false, it will return ``0`` immediately if
there is no space present in the I2S buffer.

size_t write(const uint8_t \*buffer, size_t size)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Transfers number of bytes from an application buffer to the I2S output buffer.
Be aware that ``size`` is in *bytes** and not samples.  Size must be a multiple
of **4 bytes**.  Will not block, so check the return value to find out how
many bytes were actually written.

int availableForWrite()
~~~~~~~~~~~~~~~~~~~~~~~
Returns the number of L/R samples that can be written without
potentially blocking.

int read()
~~~~~~~~~~
Reads a single sample of I2S data, whatever the I2S sample size is configured.
Will not return until data is available.

int peek()
~~~~~~~~~~
Returns the next sample to be read from the I2S buffer (without actually
removing it).

void onTransmit(void (\*fn)(void))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets a callback to be called when an I2S DMA buffer is fully transmitted.
Will be in an interrupt context so the specified function must operate
quickly and not use blocking calls like delay() or write to the I2S.

void onReceive(void (\*fn)(void))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets a callback to be called when an I2S DMA buffer is fully read in.
Will be in an interrupt context so the specified function must operate
quickly and not use blocking calls like delay() or read from the I2S.

Sample Writing/Reading API
--------------------------
Because I2S streams consist of a natural left and right sample, it is often
convenient to write or read both with a single call.  The following calls
allow applications to read or write both samples at the same time, and
explicitly indicate the bit widths required (to avoid potential issues with
type conversion on calls).

size_t write8(int8_t l, int8_t r)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Writes a left and right 8-bit sample to the I2S buffers.  Blocks until space
is available.

size_t write16(int16_t l, int16_t r)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Writes a left and right 16-bit sample to the I2S buffers.  Blocks until space
is available.

size_t write24(int32_t l, int32_t r)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Writes a left and right 24-bit sample to the I2S buffers.  See note below
about 24-bit mode.  Blocks until space is available.

size_t write32(int32_t l, int32_t r)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Writes a left and right 32-bit sample to the I2S buffers.  Blocks until space
is available.

bool read8(int8_t \*l, int8_t \*r)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Reads a left and right 8-bit sample and returns ``true`` on success.  Will block
until data is available.

bool read16(int16_t \*l, int16_t \*r)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Reads a left and right 16-bit sample and returns ``true`` on success.  Will block
until data is available.

bool read24(int32_t \*l, int32_t \*r)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Reads a left and right 24-bit sample and returns ``true`` on success.  See note below
about 24-bit mode.  Will block until data is available.

bool read32(int32_t \*l, int32_t \*r)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Reads a left and right 32-bit sample and returns ``true`` on success.  Will block
until data is available.


Note About 24-bit Samples
-------------------------
24-bit samples are stored as left-aligned 32-bit values with bits 7..0
ignored.  Only the upper 24 bits 31...8 will be transmitted or
received.  The actual I2S protocol will only transmit or receive 24 bits
in this mode, even though the data is 32-bit packed.
