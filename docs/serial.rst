Serial Ports (USB and UART)
===========================

The Arduino-Pico core implements a software-based Serial-over-USB port
using the USB ACM-CDC model to support a wide variety of operating
systems.

``Serial`` is the USB serial port, and while ``Serial.begin()`` does allow
specifying a baud rate, this rate is ignored since it is USB-based.
(Also be aware that this USB ``Serial`` port is responsible for resetting
the RP2040 during the upload process, following the Arduino standard
of 1200bps = reset to bootloader).

The RP2040 provides two hardware-based UARTS with configurable
pin selection.

``Serial1`` is ``UART0``, and ``Serial2`` is ``UART1``.

Configure their pins using the ``setXXX`` calls prior to calling ``begin()``

.. code:: cpp

        Serial1.setRX(pin);
        Serial1.setTX(pin);
        Serial1.begin(baud);

The size of the receive FIFO may also be adjusted from the default 32 bytes by
using the ``setFIFOSize`` call prior to calling ``begin()``

.. code:: cpp

        Serial1.setFIFOSize(128);
        Serial1.begin(baud);

The FIFO is normally handled via an interrupt, which reduced CPU load and
makes it less likely to lose characters.

For applications where an IRQ driven serial port is not appropriate, use
``setPollingMode(true)`` before calling ``begin()``

.. code:: cpp

        Serial1.setPollingMode(true);
        Serial1.begin(300)

For detailed information about the Serial ports, see the
Arduino `Serial Reference <https://www.arduino.cc/reference/en/language/functions/communication/serial/>`_ .


Inversion
---------

``Serial1`` and ``Serial2`` can both support inverted input and/or outputs via the methods
``Serial1/2::setInvertRX(bool invert)`` and ``Serial1/2::setInvertTX(bool invert)`` and
``Serial1/2::serInvertControl(bool invert)``.


RP2040 Specific SerialUSB methods
---------------------------------

void Serial.ignoreFlowControl(bool ignore)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In some cases, the target application will not assert the DTR virtual line, thus preventing writing operations to succeed.

For this reason, the SerialUSB::ignoreFlowControl() method disables the connection's state verification, enabling the program to write on the port, even though the data might be lost.

bool Serial.dtr()
~~~~~~~~~~~~~~~~~

Returns the current state of the DTR virtual line. A USB CDC host (such as the Arduino serial monitor) typically raises the DTR pin when opening the device, and may lower it when closing the device.

bool Serial.rts()
~~~~~~~~~~~~~~~~~

Returns the current state of the RTS virtual line.
