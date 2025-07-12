Bluetooth on PicoW Support
==========================

As of the Pico-SDK version 1.5.0, the PicoW has **BETA** Bluetooth support.

Enabling Bluetooth
------------------
To enable Bluetooth (BT), use the ``Tools->IP/Bluetooth Stack`` menu.  It
requires around 80KB of flash and 20KB of RAM when enabled.

Both Bluetooth Classic and BluetoothBLE are enabled in ``btstack_config.h``.

Included Bluetooth Libraries
----------------------------
You may use the ``KeyboardBT``, ``MouseBT``, or ``JoystickBT`` to emulate a
Bluetooth Classic HID device using the same API as their USB versions.

You may use the ``KeyboardBLE``, ``MouseBLE``, or ``JoystickBLE`` to emulate a
Bluetooth Low Energy (BLE) HID device using the same API as their USB versions.

The ``SerialBT`` library implements a very simple SPP (Serial Port Profile)
Serial-compatible port.

Connect and use Bluetooth peripherals with the PicoW using the
``BluetoothHIDMaster`` library.

``BluetoothAudio`` (A2DP) is also supported, both sink and source.

Writing Custom Bluetooth Applications
-------------------------------------
You may also write full applications using the ``BTStack`` standard callback
method, but please be aware that the Raspberry Pi team has built an
interrupt-driven version of the BT execute loop, so there is no need
to actually call ``btstack_run_loop_execute`` because the ``async_context``
handler will do it for you.

There is no need to call ``cyw43_arch_init`` in your code, either, as that
is part of the PicoW variant booting process.

For many BTStack examples, you simply need call the included
``btstack_main()`` and make sure that ``hci_power_control(HCI_POWER_ON);`` is
called afterwards to start processing (in the background).

You will also need to acquire the BT ``async_context`` system lock before
calling any BTStack APIs.  ``__lockBluetooth`` and ``unlockBluetooth`` are
provided in the PicoW variant code.

Note that if you need to modify the system ``btstack_config.h`` file, do so
in the ``tools/libpico`` directory and rebuild the Pico SDK static library.
Otherwise the change will not take effect in the precompiled code, leading
to really bad behavior.
