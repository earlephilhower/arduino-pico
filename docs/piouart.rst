"SoftwareSerial" PIO-based UART
================================

Equivalent to the Arduino SoftwareSerial library, an emulated UART using
one or two PIO state machines is included in the Arduino-Pico core.  This
allows for up to 8 additional serial ports to be run from the RP2040 without
requiring additional CPU resources.

Instantiate a ``SerialPIO(txpin, rxpin)`` object in your sketch and then
use it the same as any other serial port.  Even, odd, and no parity modes
are supported, as well as data sizes from 5- to 8-bits.

To instantiate only a serial transmit or receive unit, pass in
``SerialPIO::NOPIN`` as the ``txpin`` or ``rxpin``.

For example, to make a transmit-only port on GP16
.. code:: cpp

        SerialPIO transmitter( 16, SerialPIO::NOPIN );

For detailed information about the Serial ports, see the
Arduino `Serial Reference <https://www.arduino.cc/reference/en/language/functions/communication/serial/>`_ .
