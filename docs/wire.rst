Wire (I2C Master and Slave)
===========================

The RP2040 has two I2C devices, ``i2c0 (Wire)`` and ``i2c1 (Wire1)``

The current implementation is compatible with the Arduino standard
with the addition of two calls to allof the `SDA` and `SCL` pins
to be set **before calling Wire.begin()**

.. code:: cpp

        bool setSDA(pin_size_t sda);
        bool setSCL(pin_size_t scl);

Both master and slave operation is supported.

Master transmissions are buffered (up to 128 bytes) and only performed
on ``endTransmission``, as is standard with modern Arduino Wire implementations.

For more detailed information, check the `Arduino Wire documentation <https://www.arduino.cc/en/reference/wire>`_ .
