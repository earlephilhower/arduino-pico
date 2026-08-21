iLabs Hearth (Matter)
=====================

Matter is a smart-home standard from the Connectivity Standards Alliance
(CSA) that lets lights, sensors and appliances from different vendors join
one fabric and be driven by one controller, over WiFi, Thread or Ethernet.

The RP2040 and RP2350 have no radio of their own. On the iLabs Challenger
WiFi6 boards, Matter is spoken by a separate ESP32-C6 co-processor running
the **iLabs Hearth** firmware. This library gives you the Arduino Matter API
on the host and talks to the co-processor over a UART link, using the
iLabs ``AT+MT`` command set. The co-processor must be flashed with the
Hearth firmware for any of this to work; see `Flashing the co-processor`_
below.

::

     RP2040 / RP2350 host              ESP32-C6 co-processor
    (this Arduino library)               (Hearth firmware)
          |                                   |
          |   UART  (ESP_SERIAL_PORT)         |
          | Matter.begin()   --> AT+MTEP  --->|          BLE commissioning
          | light.setOnOff() --> AT+MTATTR -->| ))) then WiFi or Thread (((
          | Hearth.poll()    <-- +MTEVT ------|

The programming surface is source-compatible with the arduino-esp32
``Matter*`` endpoint class API. A sketch written against the arduino-esp32
core therefore builds against this library largely unmodified, with the
firmware bring-up and Tools menu described below as the only difference.
One ordering rule applies to every sketch: call ``Matter.begin()`` last,
after every endpoint object has been constructed and initialised. Endpoints
declared after ``Matter.begin()`` are not seen by the reconcile that runs
inside it.

Supported boards
-----------------

Matter over Hearth is available on the two iLabs Challenger boards that
carry an ESP32-C6 co-processor and offer the menu described below:

* Challenger RP2350 WiFi6/BLE5 (``challenger_2350_wifi6_ble5``)
* Challenger RP2040 WiFi6/BLE (``challenger_2040_wifi6_ble``)

See `Status and limitations`_ below before relying on the second one.

Choosing a firmware
--------------------

Pick the board in the IDE, open the Tools menu and set **ESP Wifi Type** to
one of the three Hearth options:

* **Hearth (Matter, WiFi)**: the usual choice. Commissioning is over BLE;
  the C6 joins WiFi on the credentials it is handed during commissioning.
* **Hearth (Matter, Thread)**: for a Thread network with a border router.
  Commissioning is still over BLE.
* **Hearth (Matter, WiFi+Thread)**: one image that can run either
  transport, selected at runtime. It carries both stacks, so it has less
  free RAM than a single-transport image; see `How many endpoints fit`_.

Every option defines ``-DILABS_HEARTH`` for the sketch build. In every
case, commissioning itself happens over BLE: the operational transport
(WiFi or Thread) only comes into play afterwards, once the controller has
handed the C6 its network credentials.

Flashing the co-processor
---------------------------

Building a sketch with a Hearth option selected only builds the sketch. The
co-processor needs its own firmware, and it is written from a terminal with
the flasher the library ships, the same way the iLabs ESP-NOW firmware is.
The IDE has no menu item for it: the Arduino platform offers a core no
action hook that fits, so the flasher is a command line tool on purpose.

The library's ``fw/`` directory holds the three prebuilt images, the
USB-to-serial bridge for the RP2040/RP2350 host, and ``flash.py``. From the
core's ``libraries/iLabs_Hearth/fw`` directory:

.. code:: sh

    python3 flash.py                  # interactive: pick WiFi, Thread or WiFi+Thread
    python3 flash.py --variant wifi   # non-interactive
    python3 flash.py --list           # verify esptool, list the variants, exit
    python3 flash.py --dry-run        # show what would happen, write nothing

Pick the variant that matches the **ESP Wifi Type** option you build the
sketch with. The flasher resets the host into its USB boot mode on its own
(no BOOTSEL press needed when exactly one board is attached, or with
``--port``), copies the bridge, flashes the C6 through it with a progress
bar, and verifies every file it wrote against ``fw/manifest.json``.

Requirements, the same as for the ESP-NOW flasher:

- **The iLabs fork of esptool.** It adds the reset profile that holds the
  C6 in its download bootloader while the RP2040/RP2350 bridges; stock
  esptool cannot flash these boards and ``flash.py`` refuses to run without
  the fork. Clone https://github.com/PontusO/esptool and point the flasher
  at it with ``ILABS_ESPTOOL_PATH`` or ``--esptool-path``.
- **pyserial** (``pip install pyserial``).
- **rich** is optional (``pip install rich``) for nicer output.

**Flashing the C6 overwrites the sketch on the host.** The bridge is a
sketch written onto the RP2040/RP2350 in place of whatever was there, so
upload your sketch again afterwards.

**The flasher works on Linux and macOS only.** It finds the host's
mass-storage mount only on those platforms, and the esptool fork skips the
bridge reset profile on Windows. Flash from a Linux or macOS machine.

Writing a sketch
------------------

A sketch declares its endpoints the same way an arduino-esp32 Matter
sketch would, then calls ``Matter.begin()`` once, last:

.. code:: cpp

    #include <Matter.h>

    MatterOnOffLight light;

    void setup() {
        Serial.begin(115200);
        light.begin();
        Matter.begin();   // reconciles the declared endpoints against the C6,
                           // called last, after every endpoint's own begin()
    }

    void loop() {
        Hearth.poll();     // services URCs from the co-processor; call every loop()
    }

The library covers 52 Matter device types in total: the twenty
arduino-esp32 ``Matter*`` endpoint classes, matching upstream one for one,
are demonstrated under ``examples/Matter*``, and the remaining
beyond-parity types the Hearth firmware also supports are demonstrated
under ``examples/FullAPI``.

How many endpoints fit
------------------------

24 is the ceiling from the Arduino IDE, and there is no way around it from
here. The firmware itself accepts up to 28 endpoints
(``MT_COMP_MAX_ENDPOINTS``), but ``HEARTH_MAX_ENDPOINTS``
(``src/MatterEndPoint.h``) sizes the host-side declaration registry at 24,
and a sketch cannot declare a 25th endpoint: the registry refuses it before
the C6 is ever asked. Raising the define needs a ``-D`` on the compiler
command line, and this core has no ``build_opt.h`` hook for a sketch folder
to add one (the ``esp32`` core has one; this one does not), so 24 is the
ceiling for any sketch built through the IDE.

On the WiFi+Thread image with WiFi as the active transport, the practical
ceiling is lower still, because that image keeps both stacks linked and
has less free RAM: roughly 20 endpoints for a typical mix of simple device
types, and roughly 12 for a mix weighted toward the energy device types.

Status and limitations
------------------------

This firmware is uncertified and uses Matter's public development
credentials (vendor ID ``0xFFF1``). Every board running it presents the
same pairing code. Consumer hubs such as Apple Home, Google Home and Alexa
are expected to refuse it, either outright or behind an
uncertified-accessory warning; that is expected of any Matter project
before its vendor pays for certification, not a defect in this firmware or
this library. A verified path to commissioning it is the NXP Matter
Chip-tool Android app, or the CLI ``chip-tool``.

Matter on ``challenger_2040_wifi6_ble`` has been compiled but never run.
The menu options and the build are offered on that board, but no hardware
run has exercised them; treat an RP2040 host as untested rather than
assuming it works the way the RP2350 does.
