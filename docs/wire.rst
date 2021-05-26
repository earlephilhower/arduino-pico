Wire (I2C Master and Slave)
===========================

The RP2040 has two I2C devices, ``i2c0 (Wire)`` and ``i2c1 (Wire1)``.

By default, `Wire` is connected to GPIO pins 4 and 5 and `Wire1` is connected
to pins 26 and 27. You may change these pins **before calling Wire.begin() or Wire1.begin()** using:

.. code:: cpp

        bool setSDA(pin_size_t sda);
        bool setSCL(pin_size_t scl);

Be sure to use pins labeled ``I2C0`` for Wire and ``I2C1`` for Wire1 on the
`Pico pinout diagram <https://datasheets.raspberrypi.org/pico/Pico-R3-A4-Pinout.pdf>`_
or it won't work.

Other than that, the API is compatible with the Arduino standard.
Both master and slave operation are supported.

Master transmissions are buffered (up to 128 bytes) and only performed
on ``endTransmission``, as is standard with modern Arduino Wire implementations.

For more detailed information, check the `Arduino Wire documentation <https://www.arduino.cc/en/reference/wire>`_ .
