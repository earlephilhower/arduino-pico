USB (Arduino and Adafruit_TinyUSB)
==================================

Two USB stacks are present in the core.  Users can choose the simpler
Pico-SDK version or the more powerful Adafruit TinyUSB library.
Use the ``Tools->USB Stack`` menu to select between the two.

Pico SDK USB Support
--------------------
This is the default mode and automatically includes a USB-based
serial port, ``Serial`` as well as supporting automatic reset-to-upload
from the IDE.

The Arduino-Pico core includes ported versions of the basic Arduino
``Keyboard`` and ``Mouse`` libraries.  These libraries allow you to
emulate a keyboard or mouse with the Pico in your sketches.

See the examples and Arduino Reference at
https://www.arduino.cc/reference/en/language/functions/usb/keyboard/
and
https://www.arduino.cc/reference/en/language/functions/usb/mouse

Adafruit TinyUSB Arduino Support
--------------------------------
Examples are provided in the Adafruit_TinyUSB_Arduino for the more
advanced USB stack.

To use Serial with TinyUSB, you must include the TinyUSB header in your
sketch to avoid a compile error.

.. code:: cpp

    #include <Adafruit_TinyUSB.h>

If you need to be compatible with the
other USB stack, you can use an ifdef:

.. code:: cpp

    #ifdef USE_TINYUSB
    #include <Adafruit_TinyUSB.h>
    #endif

Also, this stack requires sketches to manually call
``Serial.begin(115200)`` to enable the USB serial port and automatic
sketch upload from the IDE.  If a sketch is run without this command
in ``setup()``, the user will need to use the standard "hold BOOTSEL
and plug in USB" method to enter program upload mode.
