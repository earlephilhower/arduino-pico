BOOTSEL Button
==============

The BOOTSEL button on the Pico is not connected to a standard GPIO, so
it cannot be read using the usual ``digitalRead`` function.  It **can**,
however, be read using a special (relatively slow) method.

The ``BOOTSEL`` object implements a simple way of reading the BOOTSEL
button.  Simply use the object ``BOOTSEL`` as a boolean (as a conditional
in an ``if`` or ``while``, or assigning to a ``bool``):

.. code:: cpp

    // Print "BEEP" if the BOOTSEL button is pressed
    if (BOOTSEL) {
        Serial.println("BEEP!");
        // Wait until BOOTSEL is released
        while (BOOTSEL) {
            delay(1);
        }
    }