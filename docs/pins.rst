Pin Assignments
===============

The Raspberry Pi Pico has an incredibly flexible I/O configuration and most
built-in peripherals (except for the ADC) can be used on multiple sets of
pins.  Note, however, that not all peripherals can use all I/Os.  Refer to
the RP2040 datasheet or an online pinout diagram for more details.

Additional methods have been added to allow you to select a peripheral's
I/O pins **before calling ::begin**.  This is especially helpful when
using third party libraries:  the library doesn't need to be modified,
only your own code in `setup()` is needed to adjust pinouts.

I2S
---

.. code:: cpp

        ::setBCLK(pin)
        ::setDOUT(pin)

Serial1 (UART0), Serial2 (UART1)
--------------------------------

.. code:: cpp

        ::setRX(pin)
        ::setTX(pin)
        ::setRTS(pin)
        ::setCTS(pin)

SPI (SPI0), SPI1 (SPI1)
-----------------------

.. code:: cpp

        ::setSCK(pin)
        ::setCS(pin)
        ::setRX(pin)
        ::setTX(pin)

Wire (I2C0), Wire1 (I2C1)
-------------------------

.. code:: cpp

        ::setSDA(pin)
        ::setSCL(pin)


For example, because the `SD` library uses the `SPI` library, we can make
it use a non-default pinout with a simple call

.. code:: cpp

    void setup() {
        SPI.setRX(4);
        SPI.setTX(7);
        SPI.setSCK(6);
        SPI.setCS(5);
        SD.begin(5);
    }
