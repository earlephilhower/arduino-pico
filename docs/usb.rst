USB (Arduino and Adafruit_TinyUSB)
==================================

Two USB stacks are present in the core.  Users can choose the simpler
Pico-SDK version or the more powerful Adafruit TinyUSB library.
Use the ``Tools->USB Stack`` menu to select between the two.

Pico SDK USB Support
--------------------
This is the default mode and automatically includes a USB-based
serial port, ``Serial`` as well as supporting automatic reset-to-upload
from the IDE.  Note that if you ``Serial.end()`` in your code that the
reset-to-upload feature will NOT work.

The Arduino-Pico core includes ported versions of the basic Arduino
``Keyboard``, ``Mouse`` and ``Joystick`` libraries.  These libraries 
allow you to emulate a keyboard, a gamepad or mouse (or all together) 
with the Pico in your sketches.  These libraries only are available
when using the built-in USB, not the Adafruit library.

See the examples and Arduino Reference at
https://www.arduino.cc/reference/en/language/functions/usb/keyboard/
and
https://www.arduino.cc/reference/en/language/functions/usb/mouse

USB class
---------
The ``USB`` class allows for applications and libraries to add, remove,
or modify the USB connection.  Before calling any of the class functions,
be sure to include the appropriate header:

.. code::  cpp

    #include <USB.h>

Dynamic USB Configuration
-------------------------
Whenever changing the USB device information or adding or removing a device,
the USB port must be disconnected in software using ``USB.disconnect()`` to
ensure no partially-updated information gets used by the USB stack.  Once
configuration is completed calling ``USB.connect()`` will signal the PC that
a new device has been attached to the port and it should re-scan and update
the devices seen on your system.

Only the most commonly needed calls will be described in this document.  For
examples of implementing your own custom HID, please look at the ``Keyboard``
library shipped with the core.

void USB.disconnect()
~~~~~~~~~~~~~~~~~~~~
Tells the Pico to electrically disconnect from an attached USB host.
Any devices exported by the Pico should be removed in the host's OS/
This **must** be called before doing any USB modification calls.
It is safe to call this function even if the Pico isn't actually plugged
into a PC (i.e. when self-powered).

void USB.connect()
~~~~~~~~~~~~~~~~~
Tells the Pico to signal to the host that a device has been plugged
in.  The host OS should re-scan the Pico and create the new devices
exported by it.  Call this after modifying USB configurations

void USB.setVIDPID(uint16_t vid, uint16_t pid)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets the VID:PID the Pico will export to the host.  By default it will
use the settings defined for your specific board.  Be aware that Using
VIDs that are not registered by you with the USB Implementers Forum may
be problematic.  Only call after a ``usbDisconnect()``.

void USB.setManufacturer(const char *str)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sets the Manufacturer field text in the USB descriptor.  Note that most
OSes will use the ``VID`` (vendor ID) to identify the manufacturer but a
``lsusb -v`` or examination in the Windows device manager will show this
manufacturer string.   The string must be long-lived (i.e. not a local
stack stream, make it a constant or on the heap only).  Only call after
a ``usbDisconnect()``.

void USB.setProduct(const char *str)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Similar to ``usbSetManufacturer``.  Again, the OS may use the ``PID``
(product ID) to identify a device and not this string, but it will be
present on deeper inspection.  Only call after a ``usbDisconnect()``.

void USB.setSerialNumber(const char *str)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Similar to ``usbSetManufacturer``.   This is a free-form string and need
not be only numerical.  Only call after a ``usbDisconnect()``.


HID Polling Interval
--------------------
By default, HID devices will request to be polled every 10ms (i.e. 100x
per second).  If you have a higher performance need, you can override
this value by creating a global variable in your main application, set
to the polling period:

.. code::  cpp

    int usb_hid_poll_interval = 1; // Set HID poll interval to 1ms (1kHz)
    void setup() {
        ....
    }

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

Adafruit TinyUSB Configuration and Quirks
-----------------------------------------

The Adafruit TinyUSB's configuration header for RP2040 devices is stored
in ``libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/tusb_config_rp2040.h`` (`here <https://github.com/adafruit/Adafruit_TinyUSB_Arduino/blob/master/src/arduino/ports/rp2040/tusb_config_rp2040.h>`__).

In some cases it is important to know what TinyUSB is configured with. For example, by having set 

.. code:: cpp

    #define CFG_TUD_CDC 1
    #define CFG_TUD_MSC 1
    #define CFG_TUD_HID 1
    #define CFG_TUD_MIDI 1
    #define CFG_TUD_VENDOR 1

this configuration file defines the maximum number of USB CDC (serial)
devices as 1. Hence, the example sketch `cdc_multi.ino <https://github.com/adafruit/Adafruit_TinyUSB_Arduino/blob/master/examples/CDC/cdc_multi/cdc_multi.ino>`__
that is delivered with the library will not work, it will only create one
USB CDC device instead of two. It will however work when the above
``CFG_TUD_CDC`` macro is defined to 2 instead of 1.

To do such a modification when using the Arduino IDE, the file can be
locally modified in the Arduino core's package files. The base path can
be found per `this article <https://support.arduino.cc/hc/en-us/articles/360018448279-Open-the-Arduino15-folder>`__,
then navigate further to the ``packages/rp2040/hardware/rp2040/<core version>/libraries/Adafruit_TinyUSB_Arduino``
folder to find the Adafruit TinyUSB library.

When using PlatformIO, one can also make use of the feature that TinyUSB
allows redirecting the configuration file to another one if a certain
macro is set.

.. code:: cpp

    #ifdef CFG_TUSB_CONFIG_FILE
        #include CFG_TUSB_CONFIG_FILE
    #else
        #include "tusb_config.h"
    #endif

And as such, in the ``platformio.ini`` of the project, one can add

.. code:: ini

    build_flags =
      -DUSE_TINYUSB 
      -DCFG_TUSB_CONFIG_FILE=\"custom_tusb_config.h\"
      -Iinclude/ 

and further add create the file ``include/custom_tusb_config.h`` as a copy
of the original ``tusb_config_rp2040.h`` but with the needed modifications.

*Note:* Some configuration file changes have no effect because upper levels
of the library don't properly support them. In particular, even though the
maximum number of HID devices can be set to 2, and two ``Adafruit_USBD_HID``
can be created, it will not cause two HID devices to actually show up, because
of `code limitations <https://github.com/adafruit/Adafruit_TinyUSB_Arduino/blob/7264c1492a73d9a285512752b03f2550841c06bc/src/arduino/hid/Adafruit_USBD_HID.cpp#L36-L37>`__.
