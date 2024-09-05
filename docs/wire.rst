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

Note that asynchronous operations can not be intersped with normal, synchronous ones.  Fully complete or 
abort an asynchronous operation before attempting to do a normal ``Wire.beginTransaction()`` or
``Wire.requestFrom``.


bool writeReadAsync(uint8_t address, const void \*wbuffer, size_t wbytes, const void \*rbuffer, size_t rbytes, bool sendStop)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Executes a master I2C asynchronous write/read transaction to I2C slave ``address``.  First ``wbytes`` from
``wbuffer`` are written to the I2C slave, followed by an I2C restart, then ``rbytes`` are read from the
I2C slave into ``rbuffer``. The buffers need to be valid throughout the entire asynchronous operation.

At the end of the transaction an I2C stop is sent if ``sendStop`` is ``true``, and at the beginning of the
transaction an I2C start is sent if the previous write/read had ``sendStop`` set to ``true``.

Check ``finishedAsync()`` to determine when the operation completes, or use ``onFinishedAsync()`` to set a
callback.

Set ``rbytes`` to 0 to do a write-only operation, set ``wbytes`` to 0 to do a read-only operation. Or use:

``bool writeAsync(uint8_t address, const void \*buffer, size_t bytes, bool sendStop)``

``bool readAsync(uint8_t address, void \*buffer, size_t bytes, bool sendStop)``

The first call to an asynchronous write/read operation allocates the required DMA channels and internal
buffer.  If desired, call ``end()`` to free these resources.

bool finishedAsync()
~~~~~~~~~~~~~~~~~~~~
Call to check if the asynchronous operations is completed.

void onFinishedAsync(void(*function)(void))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Set a (optional) callback for async operation.  The ``function`` will be called when the asynchronous
operation finishes.

void abortAsync()
~~~~~~~~~~~~~~~~~
Cancels any outstanding asynchronous transaction.
