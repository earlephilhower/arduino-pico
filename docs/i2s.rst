I2S (Digital Audio) Output Library
==================================

While the RP2040 chip on the Raspberry Pi Pico does not include a hardware
I2S device, it is possible to use the PIO (Programmable I/O) state machines
to implement one dynamically.

This I2S library uses the `pico-extras` I2S audio library and wraps it in
an Arduino I2S library.  It supports 16 bits/sample and frequencies of up
to 48kHZ, with configurable BCLK, LRCLK(always pin "BCLK + 1"), and DOUT pins.

**Note:** This I2S device takes over the entire PIO1 (second) unit and adjusts
its clock frequency to meet the I2S needs.  That means when only 4 Tones
or only 4 Servos may be used when the I2S device is used.

bool setBCLK(pin_size_t pin)
----------------------------
Sets the BCLK pin of the I2S device.  The LRCLK/word clock will be `pin + 1`
due to limitations of the PIO state machines.  Call this before `I2S.begin()`
Default BCLK = 26, LRCLK = 27

bool setDOUT(pin_size_t pin)
----------------------------
Sets the DOUT pin of the I2S device.  Any pin may be used.  Default DOUT = 28
Call before `I2S.begin()`

bool begin(long sampleRate)
---------------------------
Start the I2S device up with the given sample rate.  The pins selected above
will be turned to output and the I2S will begin to drive silence frames (all
zero) out.

void end()
----------
Stops the I2S device.  **Note, at present the memory allocated for I2S buffers
is not freed leading to a memory leak when `end()` is called.  This is due
to the state of the `pico-extras` release from thich this device came.**

void flush()
------------
Sends any partial frames in memory to the I2S output device.  They may NOT play
immediately.  Potential use case for this call would be when the frequency of
the output will be changing.

size_t write(uint8_t)
---------------------
Provided for compatibilty, but not very useful.  Writes a sample from 0...255
to the I2S buffer.  See `write(int16_t)` for a better one

size_t write(const uint8_t \*buffer, size_t size)
-------------------------------------------------
Transfers number of bytes from an application buffer to the I2S output buffer.
Be aware that `size` is in *bytes** and not samples.  Size must be a multiple
of **4 bytes** (i.e. one left/right sample).  Will not block, so check
the return value to find out how many bytes were actually written.

int availableForWrite()
-----------------------
Returns the number of **32-bit L/R samples** that can be written without
potentially blocking.

bool setFrequency(int newFreq)
------------------------------
Adjusts the I2S output frequency.  When changing frequency while I2S output
is underway, be sure to use `I2S.flush()` before changing the frequency to
ensure the older data is played at the right frequency.


size_t write(int16_t)
---------------------
Writes a single 16-bit left or right sample to the I2S output buffer.  The
user is required to ensure that an even number of samples is written (i.e.
left and right) over the application lifetime.  The application also needs
to track which sample is next to be written (right/left).  For this reason,
the `write(void *b, size_t lrsamples)` call may be easier and faster to use.

size_t write(const void \*buffer, size_t lrsamples)
---------------------------------------------------
Writes up to size left+right packed samples to the I2S device.  Non-blocking,
will writefrom 0...size samples and return that count.  Be sure your app
handles partial writes (i.e. by yield()ing and then retrying to write the
remaining data.)

The `onTransmit` callback is not supported.

See the [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio) library
for a working example, or look at the included sample tone generator.
