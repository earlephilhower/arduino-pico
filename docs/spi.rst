SPI Master (Serial Peripheral Interface)
========================================

The RP2040 has two hardware SPI interfaces, ``spi0 (SPI)`` and ``spi1 (SPI1)``.
These interfaces are supported by the ``SPI`` library in master mode.

SPI pinouts can be set **before SPI.begin()** using the following calls:

.. code:: cpp

    bool setRX(pin_size_t pin); // or setMISO()
    bool setCS(pin_size_t pin);
    bool setSCK(pin_size_t pin);
    bool setTX(pin_size_t pin); // or setMOSI()

Note that the ``CS`` pin can be hardware or software controlled by the sketch.
When software controlled, the ``setCS()`` call is ignored.

The Arduino `SPI documentation <https://www.arduino.cc/en/reference/SPI>`_ gives
a detailed overview of the library, except for the following RP2040-specific
changes:

* ``SPI.begin(bool hwCS)`` can take an options ``hwCS`` parameter.
By passing in ``true`` for ``hwCS`` the sketch does not need to worry
about asserting and deasserting the ``CS`` pin between transactions.
The default is ``false`` and requires the sketch to handle the CS
pin itself, as is the standard way in Arduino.

* The interrupt calls (``attachInterrupt``, and ``detachInterrpt``) are not implemented.

Software SPI (Master Only)
==========================

Similar to ``SoftwareSerial``, ``SoftwareSPI`` creates a PIO based SPI interface that
can be used in the same manner as the hardware SPI devices.  The constructor takes the
pins desired, which can be any GPIO pins with the rule that if hardware CS is used then
it must be on pin ``SCK + 1``.  Construct a ``SoftwareSPI`` object in your code as
follows and use it as needed (i.e. pass it into ``SD.begin(_CS, softwareSPI);``

.. code:: cpp

    #include <SoftwareSPI.h>
    SoftwareSPI softSPI(_sck, _miso, _mosi); // no HW CS support, any selection of pins can be used

SPI Slave (SPISlave)
====================

Slave mode operation is also supported on either SPI interface.  Two callbacks are
needed in your app, set through ``SPISlave.onDataRecv`` and ``SPISlave.onDataSent``,
in order to consunme the received data and provide data to transmit.

* The callbacks operate at IRQ time and may be called very frequently at high SPI frequencies.  So, make then small, fast, and with no memory allocations or locking.


Asynchronous Operation
======================

Applications can use asynchronous SPI calls to allow for processing while long-running SPI transfers are
being performed.  For example, a game could send a full screen update out over SPI and immediately start
processing the next frame without waiting for the first one to be sent.  DMA is used to handle
the transfer to/from the hardware freeing the CPU from bit-banging or busy waiting.

Note that asynchronous operations can not be intersped with normal, synchronous ones.  ``transferAsync``
should still occur after a ``beginTransaction()`` and when ``finishedAsync()`` returns ``true`` then
``endTransaction()`` should also be called.

All buffers need to be valid throughout the entire operation.  Read data cannot be accessed until
the transaction is completed and can't be "peeked" at while the operation is ongoing.

bool transferAsync(const void \*send, void \*recv, size_t bytes)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Begins an SPI asynchronous transaction.  Either ``send`` or ``recv`` can be ``nullptr`` if data only needs
to be transferred in one direction.
Check ``finishedAsync()`` to determine when the operation completes and conclude the transaction.
This operation needs to allocate a buffer from heap equal to ``bytes`` in size if ``LSBMODE`` is used.

bool finishedAsync()
~~~~~~~~~~~~~~~~~~~~
Call to check if the asynchronous operations is completed and the buffer passed in can be either read or
reused.  Frees the allocated memory and completes the asynchronous transaction.

void abortAsync()
~~~~~~~~~~~~~~~~~
Cancels the outstanding asynchronous transaction and frees any allocated memory.


Examples
========

See the SPItoMyself and SPItoMyselfAsync examples for a complete Master and Slave application.
