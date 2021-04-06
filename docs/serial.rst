Serial Ports (USB and UART)
===========================

The Arduino-Pico core implements a software-based Serial-over-USB port
using the USB ACM-CDC model to support a wide variety of operating
systems.

`Serial` is the USB serial port, and while `Serial.begin()` doea allow
specifying a baud rate, this rate is ignored since it is USB-based.
(Also be aware that this USB `Serial` port is responsible for resetting
the RP2040 during the upload process, following the Arduino standard
of 1200bps = reset to bootloader).

The RP2040 provides two hardware-based UARTS with configurable
pin selection.

`Serial1` is UART0, and `Serial2` is UART1.

Configure their pins using the `setXXX` calls prior to calling `begin()`
.. code:: cpp

        Serial1.setRX(pin);
        Serial1.setTX(pin);
        Serial1.begin(baud);

For detailed information about the Serial ports, see the
Arduino [Serial reference](https://www.arduino.cc/reference/en/language/functions/communication/serial/)
