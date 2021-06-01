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

Master transmissions are buffered (up to 128 bytes) and only performed
on ``endTransmission``, as is standard with modern Arduino Wire implementations.

For more detailed information, check the `Arduino Wire documentation <https://www.arduino.cc/en/reference/wire>`_ .
