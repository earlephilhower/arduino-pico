Bluetooth HID Master
====================

The PicoW can connect to a Bluetooth Classic or Bluetooth BLE keyboard,
mouse, or joystick and receive input events from it.  As opposed to
the ``Keyboard``, ``Mouse``, and ``Joystick`` libraries, which make
the PicoW into a peripheral others can use, this lets the PicoW use the
same kinds of peripherals in a master rols.

BTDeviceInfo Class
------------------

The ``BluetoothHCI`` class implements a scanning function for classic
and BLE devices and can return a ``std::vector`` of discovered devices to an application.
Iterate over the list using any of the STL iteration methods (i.e. ``for (auto e : list)``).
The elements of this list are ``BTDeviceInfo`` objects which have the following
member functions:

``BTDeviceInfo::deviceClass()`` returns the Bluetooth BLE UUID or the Blustooth device
class for the device.  This specifies the general class of the device (keyboard, mouse,
etc.).

``BTDeviceInfo::address()`` and ``BTDeviceInfo::addressString()`` return the
Bluetooth address as a binary array or a string that can be used to ``print``.

``BTDeviceInfo::addressType()`` returns whether the BLE address is random or not, and
is not generally needed by a user application.

``BTDeviceInfo::rssi()`` returns an approximate dB level for the device.  Less
negative is stronger signal.

``BTDeviceInfo::name()`` returns the advertised name of the device, if present.  Some
devices or scans do not return names for valid devices.


BluetoothHCI Class
------------------

The ``BluetoothHCI`` class is responsible for scanning for devices and the lower-level
housekeeping for a master-mode Bluetooth application.  Most applications will not need
to access it directly, but it can be used to discover nearby BT devices.  As
part of the application Bluetooth setup, call ``BluetoothHCI::install()`` and then
``BluetoothHCI::begin()`` to start BT processing.  Your application is still responsible
for all the non-default HCI initialization and customization.  See the ``BluetoothScanner.ino``
example for more info.


BluetoothHIDMaster Operation
----------------------------

Most applications will use the ``BluetoothHIDMaster`` class and, after connecting, receive
callbacks from the Bluetooth stack when input events (key presses, mouse moves, button
mashes) occur.

Application flow will generally be:
1. Install the appropriate callbacks using the ``BluetoothHIDMaster::onXXX`` methods
2. Start the Bluetooth processing with ``BluetoothHIDMaster::begin()`` or ``BluetoothHIDMaster::beginBLE()``
3. Connect to the first device found with ``BluetoothHIDMaster::connectXXX()`` and start receiving callbacks.
4. Callbacks will come at interrupt time and set global state variables, which the main ``loop()`` will process

Callback Event Handlers
-----------------------
The main application is informed of new inputs via a standard callback mechanism.  These callbacks run at
interrupt time and should not do significant work, ``delay()``, or allocate or free memory.  The most common
way of handling this is setting global ``volatile`` flags and variables that the main ``loop()`` will poll
and process.

Mouse Callbacks
~~~~~~~~~~~~~~~
The ``BluetoothHIDMaster::onMouseMove`` callback gets the delta X, Y, and wheel reported by the device.
The ``BluetoothHIDMaster::onMouseButton`` gets a button number and state (up or down) and will be called
each time an individual button is reported changed by the mouse.

.. code :: cpp

    void mouseMoveCB(void *cbdata, int dx, int dy, int dw) {
        // Process the deltas, adjust global mouse state
    }

    void mouseButtonCB(void *cbdata, int butt, bool down) {
        // Set the global button array with this new info
    }


Meyboard Callbacks
~~~~~~~~~~~~~~~~~~
The `BluetoothHIDMaster::onKeyDown`` callback receives the raw HID key (**NOT ASCII**) sent by the device on a key press
while `BluetoothHIDMaster::onKeyUp`` gets the same when a key is released.  Note that up to 6 keys can be pressed at any
one time.  For media keys ("consumer keys" in the USB HID documentation) the ``BluetoothHIDMaster::onConsumerKeyDown`` and
``BluetoothHIDMaster::onConsumerKeyUp`` perform the same function (and receive the consumer key IDs defined by the
USB HID spec and not ASCII).

To convert the key press and release (including SHIFT handling), use a ``HIDKeyStream`` object.  Simply write the raw
HID key and the up/down state to the stream and read back the ASCII for use in an application.

.. code :: cpp

    HIDKeyStream keystream;

    void keyDownCB(void *cbdata, int key) {
        keystream.write((uint8_t )key);
        keystream.write((uint8_t) true); // Keystream now has 1 ASCII character to read out and use
        char ascii = keystream.read();
        // ....
    }

    void keyUpCB(void *cbdata, int key) {
        // Normally don't do anything on this, the character was read in the keyDownCB
    }

    void consumerKeyDownCB(void *cbdata, int key) {
        // switch(key) and use cases from the USB Consumer Key page
    }

    void consumerKeyUpCB(void *cbdata, int key) {
        // switch(key) and use cases from the USB Consumer Key page
    }


Joystick Callbacks
~~~~~~~~~~~~~~~~~~
A single ``BluetoothHIDMaster::onJoystick`` callback gets activated every time a report from a joystick is processed.
It receives (potentially, if supported by the device) 4 analog axes, one 8-way digital hat switch position, and up
to 32 button states at a time.

.. code :: cpp

    void joystickCB(void *cbdata, int x, int y, int z, int rz, uint8_t hat, uint32_t buttons) {
        // HAT 0 = UP and continues clockwise.  If no hat direction it is set to 0x0f.
        // Use "buttons & (1 << buttonNumber)" to look at the individual button states
        // ...
    }

PianoKeyboard Example
~~~~~~~~~~~~~~~~~~~~~
See the ``PianoKeyboard.ino`` and ``PianoKeyboardBLE.ino`` examples for more information on callback operation.


BluetoothHIDMaster::onXXX Callback Installers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code :: cpp

    void BluetoothHIDMaster::onMouseMove(void (*)(void *, int, int, int), void *cbData = nullptr);
    void BluetoothHIDMaster::onMouseButton(void (*)(void *, int, bool), void *cbData = nullptr);
    void BluetoothHIDMaster::onKeyDown(void (*)(void *, int), void *cbData = nullptr);
    void BluetoothHIDMaster::onKeyUp(void (*)(void *, int), void *cbData = nullptr);
    void BluetoothHIDMaster::onConsumerKeyDown(void (*)(void *, int), void *cbData = nullptr);
    void BluetoothHIDMaster::onConsumerKeyUp(void (*)(void *, int), void *cbData = nullptr);
    void BluetoothHIDMaster::onJoystick(void (*)(void *, int, int, int, int, uint8_t, uint32_t), void *cbData = nullptr);

BluetoothHIDMaster Class
------------------------

bool BluetoothHIDMaster::begin()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Installs and configures the Bluetooth Classic stack and starts processing events.  No connections are made at this point.
When running in Classic mode, no BLE devices can be detected or used.


bool BluetoothHIDMaster::begin(const char *BLEName)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Installs and configures the Bluetooth BLE stack and starts processing events.  No connections are made at this point.
When running in BLE mode, no Classic devices can be detected or used.

bool BluetoothHIDMaster::connected()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Returns if the Bluetooth stack is up and running and a connection to a device is currently active.

void BluetoothHIDMaster::end()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Disables the Bluetooth stack.  Note that with the current Bluetooth implementation restarting the stack (i.e. calling ``begin()`` after ``end()``) is not stable and will not work.  Consider storing state and rebooting completely if this is necessary.

bool BluetoothHIDMaster::running()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Returns if the Bluetooth stack is running at all.  Does not indicate if there is an active connection or not.

bool BluetoothHIDMaster::hciRunning()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Returns if the Bluetooth stack has passed the initial HCI start up phase.  Until this returns ``true`` no Bluetooth operations can be performed.

std::vector<BTDeviceInfo> BluetoothHIDMaster::scan(uint32_t mask, int scanTimeSec, bool async)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Passes through the ``BluetoothHCI::scan()`` function to manually scan for a list of nearby devices.  If you want to connect to the first found device, this is not needed.

bool BluetoothHIDMaster::connect(const uint8_t *addr)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Start the connection process to the Bluetooth Classic device with the given MAC.  Note that this returns immediately, but it may take several seconds until ``connected()`` reports that the connection has been established.

bool BluetoothHIDMaster::connectKeyboard(), connectMouse(), connectJoystick(), connectAny()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Connect to the first found specified Bluetooth Classic device type (or any HID device) in pairing mode.  No need to call ``scan()`` or have an address.

bool BluetoothHIDMaster::connectBLE(const uint8_t *addr, int addrType)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Start the connection process to the Bluetooth BLE device with the given MAC.  Note that this returns immediately, but it may take several seconds until ``connected()`` reports that the connection has been established.

bool BluetoothHIDMaster::connectBLE()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Connect to the first found BLE device that has a HID service UUID (keyboard, mouse, or joystick)

bool BluetoothHIDMaster::disconnect()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Shuts down the connection to the currently connected device.

void BluetoothHIDMaster::clearPairing()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Erases all Bluetooth keys from memory.  This effectively "forgets" all pairing between devices and can help avoid issues with the beta Bluetooth stack in the SDK.
