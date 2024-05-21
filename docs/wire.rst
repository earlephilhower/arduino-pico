Wire (I2C Master and Slave)
===========================

The RP2040 has two I2C devices, ``i2c0 (Wire)`` and ``i2c1 (Wire1)``.

The default pins for `Wire` and `Wire1` vary depending on which board you're using.
(Here are the pinout diagrams for `Pico <https://datasheets.raspberrypi.org/pico/Pico-R3-A4-Pinout.pdf>`_
and `Adafruit Feather <https://learn.adafruit.com/assets/100740>`_.)

You may change these pins **before calling Wire.begin() or Wire1.begin()** using:

.. code:: cpp

        bool setSDA(pin_size_t sda);
        bool setSCL(pin_size_t scl);

Be sure to use pins labeled ``I2C0`` for Wire and ``I2C1`` for Wire1 on the pinout
diagram for your board, or it won't work.

Other than that, the API is compatible with the Arduino standard.
Both master and slave operation are supported.

Master transmissions are buffered (up to 256 bytes) and only performed
on ``endTransmission``, as is standard with modern Arduino Wire implementations.

For more detailed information, check the `Arduino Wire documentation <https://www.arduino.cc/en/reference/wire>`_ .

Asynchronous Operation
----------------------

Applications can use asynchronous I2C calls to allow for processing while long-running I2C operations are
being performed.  For example, a game could send a full screen update out over I2C and immediately start
processing the next frame without waiting for the first one to be sent over I2C.  DMA is used to handle
the transfer to/from the I2C hardware freeing the CPU from bit-banging or busy waiting.

Note that asynchronous operations can not be intersped with normal, synchronous ones.  Fully complete an
asynchronous operation before attempting to do a normal ``Wire.beginTransaction()`` or ``Wire.requestFrom``.
Also, all buffers need to be valid throughout the entire operation.  Read data cannot be accessed until
the transaction is completed and can't be "peeked" at while the operation is ongoing.


bool writeAsync(uint8_t address, const void \*buffer, size_t bytes, bool sendStop)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Begins an I2C asynchronous write transaction.  Writes to ``address`` of ``bytes`` from ``buffer`` and
at the end will send an I2C stop if ``sendStop`` is ``true``.
Check ``finishedAsync()`` to determine when the operation completes and conclude the transaction.
This operation needs to allocate a buffer from heap equal to 2x ``bytes`` in size.

bool readAsync(uint8_t address, void \*buffer, size_t bytes, bool sendStop)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Begins an I2C asynchronous read transaction.  Reads from ``address`` for ``bytes`` into ``buffer`` and
at the end will send an I2C stop if ``sendStop`` is ``true``.
Check ``finishedAsync()`` to determine when the operation completes and conclude the transaction.
This operation needs to allocate a buffer from heap equal to 4x ``bytes`` in size.

bool finishedAsync()
~~~~~~~~~~~~~~~~~~~~
Call to check if the asynchronous operations is completed and the buffer passed in can be either read or
reused.  Frees the allocated memory and completes the asynchronous transaction.

void abortAsync()
~~~~~~~~~~~~~~~~~
Cancels the outstanding asynchronous transaction and frees any allocated memory.

