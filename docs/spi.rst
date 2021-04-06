SPI (Serial Peripheral Interface)
=================================

The RP2040 has two hardware SPI interfaces, ``spi0 (SPI)`` and ``spi1 (SPI1)``.
These interfaces are supported by the ``SPI`` library in master mode.

SPI pinouts can be set **before SPI.begin()** using the following calls:

.. code:: cpp

    bool setRX(pin_size_t pin);
    bool setCS(pin_size_t pin);
    bool setSCK(pin_size_t pin);
    bool setTX(pin_size_t pin);

Note that the ``CS`` pin can be hardware or software controlled by the sketch.
When software controlled, the ``setCS()`` call is ignored.

The Arduino `SPI documentation <https://www.arduino.cc/en/reference/SPI>`_ gives
a detailed overview of the library, except for the following RP2040-specific
changes:

* ``SPI.begin(bool hwCS)`` can take an options ``hwCS`` parameter.  By passing in ``true`` for ``hwCS`` the sketch does not need to worry about asserting and deasserting the ``CS`` pin between transactions.  The default is ``false`` and requires the sketch to handle the CS pin itself, as is the standard way in Arduino.
* The interrupt calls (``usingInterrupt``, ``notUsingInterrupt``, ``attachInterrupt``, and ``detachInterrpt``) are not implemented.
