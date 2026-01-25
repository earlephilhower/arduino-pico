BLE (Bluetooth Low Energy) Server/Client
========================================

Support for using the Pico W as a BLE server (peripheral) or BLE
client (central) is available through the BLE library.

Note: These instructions assume knowledge about BT and BLE in
general.  Look at the official ArduinoBLE documentation for a
quick overview of the basics.

This library was developed specifically for the BT implementation
used in the Pico SDK, BTStack.  Its API is close to the ESP32 BLE
and the official ArduinoBLE, but not directly copmatible with either.
However, if you have used either of those libraries then coming up
to speed on Arduio-Pico BLE should be straightforward.

To use this library, include BLE.h in your sketch and ensure the
``Tools->IP/BT Stack`` option in the menus has ``+ Bluetooth``
enabled (and, obviously, you need a BT-enabled Pico or compatible
board with RM2 coprocessor).

.. code:: cpp

    #include <BLE.h>
    ...
    void setup() {
        // Optionally set security mode, before .begin, see below
        BLE.begin(); // Initialize the BT stack
        ...
    }

This library is not compatible with FreeRTOS due to memory allocations
required at interrupt time.

General Architecture
--------------------

BLE implements a hierarchical object structure, with a top-level
``BLE`` containing either a ``BLEServer`` or a ``BLEClient``.
``BLE`` handles scanning for devices to connect to in ``BLEClient`` mode,
but other than that all work is handled by the ``BLEServer`` or
``BLEClient`` as appropriate.

Applications can't generate their own ``BLEServer`` or ``BLEClient``
objects, they must use the ``BLE.server()`` or ``BLE.client()`` calls
to get a pointer to the global server or client object.

.. code:: cpp

    #include <BLE.h>
    void setup() {
        BLE.begin();
        auto server = BLE.server(); // Get pointer to single global server object
        // ... create a BLEService with BLECharacteristics somehow...
        server->addService(...); // Add the service we just made to the object
        server->startAdvertising(); // Tell the world about it!
    }

``BLEServer`` holds a collection of application-defined ``BLEService``
objects which represent a BLE service.  Each ``BLEService`` can contain
a list of ``BLECharacteristics`` which are what the host PC or phone
will actually interact with.  Each ``BLECharacteristic`` contains a
application-defined value which can be read or written by the host PC.
The sketch can be notified when the host PC or phone writes to one of
its characteristics (i.e. a callback could set the dimming of a LED
depending on the value written by the PC).

``BLEClient`` connects to a remote BLE server (peripheral) and creates
a set of ``BLERemoteService`` objects, each of which can contain zero of
more ``BLERemoteCharacteristic`` objects that the remote device supports.
Each of those ``BLERemoteCharacteristic`` can be read or written (if
allowed) and can be optionally notified when the remote changes through
the ``onNotify`` callback (again, if the remote server allows it).

Sketches operate with the BLE at two levels: main application and
BLE-event triggered callbacks.  The main app can write or read a
characteristic at any time.  Callbacks happen when the remote end of
the connection performs a read or write to a characteristic and execute
at interrupt-level (so no filesystem access, etc.).


General Bluetooth Support Classes
=================================


BLEAddress
----------

This class holds a BLE MAC address.  As of the initial release, only
public addresses are supported.  Randomized ones are not yet, but if
they're important to you please submit a PR.

BLEAddress(uint8_t \*a)
~~~~~~~~~~~~~~~~~~~~~~~

Creates a ``BLEAddress`` using the 6-byte MAC of a device.

.. code :: cpp

    uint8_t mySpecialDevice[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    BLEAddress x(mySpecialDevice);


BLEAddress(String a)
~~~~~~~~~~~~~~~~~~~~

Creates a ``BLEAddress`` by parsing the MAC in human readable form
(hex separated with colons)

.. code :: cpp

    BLEaddress x("01:02:03:04:05:06");


BLEUUID
-------

All services and characteristics are identified by UUIDs in BLE.  There are
many predefined 16-bit UUIDs for common devices such as exercise equipment or
home monitoring, but anyone can create their own 128-bit UUID for a customized
service and characteristics.

BLEUUID(uint16_t u)
~~~~~~~~~~~~~~~~~~~

Create a 16-bit UUID.  See `Assigned Numbers <https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf>`__
for the values predefined by the Bluetooth SIG.

.. code :: cpp

    BLEUUID batteryServiceUUID(0x180F);

BLEUUID(const uint8_t u[16])
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a 128-bit UUID (16 bytes) from an array of bytes

.. code :: cpp

    uint8_t mySuperCool[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    BLEUUID mySuperCoolUUID(mySuperCool);

BLEUUID(String u)
~~~~~~~~~~~~~~~~~

Creates a 128-bit UUID from a human-readable string

.. code :: cpp

    BLEUUID myNewUUID("040b4668-cae0-4b57-88c6-a749d7b1e3dd");


BLE (Bluetooth Low Energy) Base
===============================

The ``BLE`` object is how a sketch interacts with the hardware Bluetooth stack.
It manages the low-level BTStack and contains either a ``BLEClient`` or ``BLEServer``
where the actual BLE operations will happen.

Configuring Security and Starting Up
------------------------------------

Before calling ``BLE.begin()`` you can specify if you want the Pico to request
a bonding with "Just Works" security (used for things like mice or headsets where
there really isn't a good user input/output capability) or allow any device to
connect without building a connection (the default)

BLE.setSecurity(``BLESecurityNone`` or ``BLESecurityJustWorks``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Configures the BLE security model.  By default no pairing will be needed and
devices will be able to interact with any services shared on this Pico.
``BLESecurityJustWorks`` enables PIN-less pairing with a BLE central/client
which enables encryption (but not authentication).

This must be called **before** ``BLE.begin()`` is executed

BLE.begin(String name)
~~~~~~~~~~~~~~~~~~~~~~

Starts the BLE stack on the Pico.  Call it before setting up any services or
client operations.  The ``name`` is used when advertising the device.

BLEAddress BLE.address()
~~~~~~~~~~~~~~~~~~~~~~~~

Returns this Pico's BLE MAC address.  Only valid after ``BLE.begin()``.


Accessing the Global BLEServer or BLEClient
-------------------------------------------

To get a pointer to either the server or client objects to actually do
work, call the appropriate ``BLE`` method

BLEServer \*BLE.server()
~~~~~~~~~~~~~~~~~~~~~~~~

Returns the global server object which will contain a list of ``BLEService`` objects
that will be used by the outside world to connect to this Pico.  See the Server section below
for more information.

BLEClient \*BLE.client()
~~~~~~~~~~~~~~~~~~~~~~~~

Returns the global client object which will be used to connect to a remore BLE server,
remote services, and remove characteristics.  See the Client section below

Configuring BLE Advertising
---------------------------

Advertising is the way that a BLE device shares its presence to the world.  The BLE hardware will
broadcast a small (31 byte) packet containg information cush ad the device name, what it looks
like, and any service UUIDs it provides.  BLE clients normally don't need to advertise and can
ignore the following information.

BLEAdvertising \*BLE.advertising()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This call returns the current advertising object for any defined servers.  In general,
sketches do not need to work with this call or ``BLEAdvertising`` at all, but this
call lets you do things like add URLs or appearance information.

BLE.startAdvertising()
~~~~~~~~~~~~~~~~~~~~~~

After all services are created (see below) the sketch needs to use this call to start broadcasting
for BLE clients to find it.  Call this after all services are defined, or after ``BLE.stopAdvertising()``
and changing services or characteristics.  Without this call, your Pico won't be discoverable.

BLE.stopAdvertising()
~~~~~~~~~~~~~~~~~~~~~

Turns off advertising to allow modifying services/etc.


Finding BLE Servers to Connect To
---------------------------------

BLE clients need to listen the BLE advertisements around them to discover servers they can
connect to.  This library allows for scanning for all servers, or only servers exposing a
service they care about (i.e. only discovering available thermostats or heart rate monitors).

BLEScanReport \*scan(int timeoutSec = 5, bool active = true, int intervalms = 100, int windowms = 99)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Listens to the BLE announcements for ``timeoutsec`` seconds and returns a ``BLEScanReport`` which is
a ``std::list<BLEAddress>`` of all seen BLE servers available.  Enteires in this list can be
used in a ``BLE.client()->connect()`` call to initiate a BLE client connection (see below).

BLEScanReport *scan(BLEUUID service, int timeoutSec = 5, bool active = true, int intervalms = 100, int windowms = 99)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Like the prior call, except only returns servers who are advertising the service UUID specified.
Most sketches will want to use this call to look for only the device types they want (i.e.
``auto cyclingInfoList = BLE.scan(BLEUUID(0x1816)); // Only look for cycling devices`` )

BLE.clearScan()
~~~~~~~~~~~~~~~

The information for each BLE device seen is stored in memory, taking around 100 bytes of memory per
device.  If sketches are very memory constrained, call this method to free all the memory associated
with the last ``BLE.scan()`` after you have selected a single device to connect to.  Most sketches
don't need to do this.

Implementing a BLE Server (peripheral/device)
=============================================

A BLE gadget/peripheral/device offers zero or more ``BLEService`` to the outside world.
Each ``BLEService`` contains zero or more ``BLECharacteristics`` that the outside world can
read and/or write to.  The objects defining these services and characteristics must be
created before calling ``BLE.startAdvertising()``.

BLEService
==========

A ``BLEService`` is essentially a container to hold a list of ``BLECharacteristics`` identified
by a ``BLEUUID`` that identifies the type of service.

For well known, predefined services
these UUIDS are standardized by the Bluetooth SIG.  If you are implementing a gadget that
provides predefined services, using the well known service UUIDs (see section 3.4.1 in
`Assigned Numbers <https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf>`__
will allow any BLE client device (PC, phone, home hub) to interact with the Pico.

You can (and should) generate your own UUID (``uuidgen`` on Linux or any online generator)
when creating custom services.

Create a BLEService with the UUID using ``new`` in a function

.. code :: cpp

    void setup() {
        BLE.begin();
        BLEService *cyclingPowerService = new BLEService(BLEUUID(0x1818)); // 0x1818 defined by SIG as "Cycling Power Service"
        ...

or using a global variable (because you need the ``BLEService`` object to live for the entire
application, you can't take the address of an immediate local variable...it won't be there
after the function exits)

.. code :: cpp

    BLEService cyclingPowerService = BLEService(BLEUUID(0x1818)); // 0x1818 defined by SIG as "Cycling Power Service"

    void setup() {
        BLE.begin();
        ...

Adding Characteristics to a BLEService
======================================

Once you have a ``BLEService`` you need to create ``BLECharacteristics`` to share data
(i.e. current temperature, running speed, solar power, etc.) with a client.  They can
be though of as a free-form variable that both your sketch and the outside world can
read (and update if permitted).

Characteristics have a UUID to identify them (again, there are well-known ones for
well-known services and you should use them if appropriate, or generate your own
for custom ones you generate).

Note that for custom UUIDs, the UUID used for characteristics should not be the same
as the UUID for the custom service.  Generate new UUIDs for every characteristic or
it could confuse BLE clients.

Create a characteristic with the following constructor (again, be sure it is a "long
lived" variable to make it global or use ``new`` to make it on the heap

BLECharacteristic(BLEUUID u, uint16_t characteristicPermission, const char *desc = nullptr, uint8_t permR = 0, uint8_t permW = 0)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Creates a characteristic with the given UUID.

Permissions is the binary OR (``|``) of ``BLERead``, ``BLEWrite``, ``BLENotify``
(i.e. ``BLERead | BLENotify`` or ``BLERead | BLEWrite``) where ``BLERead`` means the client (and the sketch) can read the value
(i.e. it is an output of your sketch), ``BLEWrite`` means the client can write the value (the sketch can, too, of course),
and ``BLENotify`` means the Pico will let the client know when the Pico changes it with ``setValue()`` , via the BLE notify message.

The optional ``desc`` parameter is a human-readable text description of the characteristic.  It is not mandatory and only certain
BLE tools can read and display it.

.. code :: cpp

    void setup() {
        BLE.begin();
        BLEService *cyclingPowerService = new BLEService(BLEUUID(0x1818)); // 0x1818 defined by SIG as "Cycling Power Service"
        BLECharacteristic *pwrChar = new BLECharacteristic(BLEUUID(0x2A63), BLEREAD, "How hard am I working"); // 0x2a64 is "Cycling Power Measurement"

Once a characteristic has been created, add it to a service.  Services can have many characteristics, and the
order in which they are added is not generally important since they are identified by the UUID, not the index.

Keep the characteristic variable pointer handy to set values to it or receive callbacks when remote
reads or writes happen.


BLEService::addCharacteristic(BLECharacteristic \*c)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adds the characteristic to the service.  As always, the variable needs to be on the heap or a global to
ensure the pointer remains valid the lifetime of the sketch.

Setting Characteristic Data
===========================

Characteristics can be read and written to by the sketch (and the remote client if permissions allow it).
They can be any format desired (character strings, binary floating point, 1-byte booleans), but for well-known
services be sure to use the format specified by the Bluetooth SIG.

When values are set by the application, if the BLE client requests (and is permitted) notification, they
will automatically get a message with the new data, the sketch itself needs do nothing.

BLECharacteristic::setValue(<simple data type>)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The characteristic object can set by the sketch by calling this function.  The single value passed in
will be stored in the characteristic in the same format as passed-in (i.e. ``setValue((uint16_t)0xabcd)``
would set the characteristic to 2 bytes of data, {0xab, 0xcd}).

BLECharacteristic::setValue(const uint8_t \*data, size_t size)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sets the characteristic to any binary data (i.e. a large ``struct``) you pass in, at any size.  The data is copied over so the passed-in
pointer can be invalidated by the sketch at any time.

Reading Characteristic Data
===========================

The BLE client (PC, phone, etc.) can write data to characteristics (if permissions allow) and the sketch
can read those values and do things with them (i.e. set a thermostat, light an LED, etc.).  Use
the appropriate ``BLECharacteristic::get<type>`` call.

.. code :: cpp

    bool getBool()
    char getChar()
    uint8_t getInt8()
    unsigned char getUChar()
    uint8_t getUInt8()
    short getShort()
    int16_t getInt16()
    unsigned short getUShort()
    uint16_t getUInt16()
    int getInt()
    int32_t getInt32()
    unsigned int getUInt()
    uint32_t getUInt32()
    long getLong()
    unsigned long getULong()
    long long getLongLong()
    int64_t getInt64()
    unsigned long long getULongLong()
    uint64_t getUInt64()
    float getFloat()
    float getDouble()
    String getString()

If you need the raw binary data use the following calls

.. code :: cpp

    const void * valueData()
    size_t valueLen()

Getting Callbacks for Characteristics
=====================================

Your sketch can get a callback to a function or class object whenever
the remote devices reads or writes a characteristic (i.e. you could configure
a callback whenever the home hub writes a new temperature setting to your
device instead of polling ``characteristic->getFloat()``).

To register a read or write callback (which will be executed at interrupt level
so don't do things like filesystem access or flash writes) use

.. code :: cpp

    void myLEDReadCB(BLECharacteristic *c) {
        Serial.println("Yay, someone read me!");
    }

    void myLEDWriteCB(BLECharacteristic *c) {
        // Control the LED, immediately
        digitalWrite(LED_BUILTIN, c->getBool());
    }

    void setup() {
        BLE.begin("LEDMASTER");
        ... set up service
        auto s = new BLEService(BLEUUID(MYCUSTOMSVC));
        auto c = new BLECharacteristic(BLEUUID(MYCUSTOMUUID), BLEWrite | BLEREAD, "Lightbulb Control");
        c->onRead(myLEDReadCB);
        c->onWrite(myLEDWriteCB);
        s->addCharacteristic(c);
        BLE.startAdvertising();
    }

    void loop() {
        // Nothing here, the LED will be controlled thru the callback!
    }

For a class-based callback demonstration, see the BLEServiceUART.cpp source file.

Implementing a BLE Client (BLE Central)
=======================================

A BLE client scans the available device and then connects to one that offers the
service it needs.  When it connects it then gets a full list of zero or more remote services.
For each of those remote services it then gets a full list of remote characteristics it
can use to interact with the device.

Once a list of available servers is collected via ``BLE.scan()`` as explained above, the
Pico chooses one to connect to with a call to

bool BLE.client()->connect(BLEAdvertising a, int timeoutsecs = 10)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Attempts to connect to a remote BLE server and perform the scans described above.  If the
connection doesn't complete within the timeout, it will fail and return ``false`` .
Note that further BLE operations to that server will use a different timeout (default 2 
seconds) which can be changed via ``BLE.client()->setTimeout(int timeoutsecs)``

Accessing Remote Services
=========================

After successful connections, it is possible to examine the list of remote services or
choose any specific one (most common)

BLERemoteService \*BLE.client()->service(BLEUUID)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Returns either `nullptr` if no service exists, or ``BLERemoteService`` that can be used
to find characteristics the sketch cares about.  Don't ``delete`` this object, it is managed
completely by the ``BLE`` object.

Accessing Remote Characteristics
================================

Remote characteristics can be read, written, or registered for notification (remote permissions
allowing it, of course).  All remote characteristics are wrapped in ``BLERemoteCharacteristic`` objects.

There is no built-in caching, so every read of a remote characteristic goes over the BLE
radio and can take some time. Your sketch should cache any values it needs to use repeatedly.

Remote Characteristic Permissions
---------------------------------

The BLE server defines the permissions the Pico is given for each of the remote characteristics
it provides.

.. code :: cpp

    bool BLERemoteCharacteristic::canRead()
    bool BLERemoteCharacteristic::canWrite()
    bool BLERemoteCharacteristic::canNotify()

String BLERemoteCharacteristic::description()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Returns the human readable text description associated with the remote characteristic,
if the remote server supplies one.  This is the equivalent of the descriptino parameter
in the BLECharacteristic constructor.

Reading Remote Characteristics
==============================

The same ``getXXX`` calls as defined for ``BLECharacteristic`` (local characteristic)
work for ``BLERemoteCharacteristic`` .  They may be somewhat slower due to the need to
poll the remote server for the data.

Writing Remote Characteristics
==============================

The same ``setXXX`` calls as defined for ``BLECharacteristic`` work for ``BLERemoteCharacteristic``
except, of course, that they send the new data to the remote device.

Getting Callbacks for Remote Characteristics
============================================

Your sketch can receive notifications when the remote server writes to a characteristic,
assuming the remote server gives permission.  That avoids having to read a remote characteristic
over and over (using up radio time and power) to check for changes.

Both class and functional callbacks are available.  Class-based works the same as for
``BLECharacteristics`` .

BLERemoteCharacteristic::onNotify(void (\*notifyCB)(BLERemoteCharacteristic \*c, const uint8_t \*data, uint32_t len))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Registers a callback that will be called ar interrupt level when a BLE notify message is
received for the remote characteristic.  Your callback will need to cast the returned ``data`` pointer
to the appropriate format (i.e. a ``bool`` or a ``char[]``).


BLE Beacons
===========

BLE beacons are devices which just advertise their presence (infrequently, to save power)
and provide no services or characteristics.  All information about them (their UUID and
their major and minor ID numbers) are contained in the advertisement.

The ``BLEBeacon`` class implements this functionality.  Note that no ``BLE..service()`` calls
should be used in this mode, just ``BLEBeacon::begin()`` (after setting the UUID and major/minor
IDs.  See the ``Beacon.ino`` example for usage


BLE Battery Service
===================

The Bluetooth SIG standard BLE Battery service is implemented in ``BLEServiceBattery``.  Instantiate
an instance of this object and ``BLE.server()->addService()`` it to gain this functionality.

Call ``BLEBatteryService::set(int lvl /* 0-100 */)`` from your main sketch as the battery state changes.
All other work is handled behind-the-scenes.

This service can be added alongside other services, of course.

BLE Serial UART (Nordic SPP Service)
====================================

While not a Bluetooth SIG standard, the BLE "UART" service created by Nordic for their chipsets
is very commonly used for low-power, low-volume data transmission over BLE.  The
``BLEServiceUART`` class implemnts this "standard" as a ``arduino::HardwareSerial`` object
(like Serial or Serial1/2 or SoftwareSerial).  This service can be combined with others, as
always.  It implements an automatic data flush mechanism, or the user can ``flush()`` as
appropriate for their protocol.

See the ``SerialBLE.ino`` example for usage.
