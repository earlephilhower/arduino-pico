WiFi (Raspberry Pi Pico W) Support
==================================

WiFi is supported on the Raspberry Pi Pico W by selecting the "Raspberry Pi Pico W" board in the Boards Manager.  It is generally compatible with the `Arduino WiFi library <https://www.arduino.cc/en/Reference/WiFi>`__ and the `ESP8266 Arduino WiFi library <https://github.com/esp8266/Arduino>`__.

Enable WiFi support by selecting the `Raspberry Pi Pico W` board in the IDE and adding ``#include <WiFi.h>`` in your sketch.

Non-Raspberry Pi WiFi Support
-----------------------------

In addition to the official WiFi chip from Raspberry Pi, this core supports additional WiFi interface devices:

* Microchip WINC1500

* ESP32-based devices using `ESPHost <https://github.com/Networking-for-Arduino/ESPHost>`__

See the examples in the respective library folders for instructions on instantiating and using these devices.  Most of the features documented here will also work on these WiFi chips.


Supported Features
------------------

* WiFi connection (Open, WPA/WPA2) 

  * Static IP or dynamic DHCP supported

  * Station Mode (STA, connects to an existing network)

  * Access Point Mode (AP, creates its own wireless network) with 4 clients

* WiFi Scanning and Reporting
 
  * See the ``ScanNetworks.ino`` example to better understand the process.


Powering the WiFi Chip Off (Pico W / Pico 2 W)
----------------------------------------------

``WiFi.end()`` and ``WiFi.disconnect()`` leave the network but keep the CYW43 chip powered, which costs about 20 mA
on the board whatever the CPU does (about 50 mA with the station connected and idle).  To save power while the
WiFi is not needed (a sleep of more than about 10 seconds, see below) power the chip off:

.. code:: cpp

    bool WiFi.powerOff();
    bool WiFi.powerOn(bool reconnect = true, uint32_t timeoutMs = 15000);
    bool WiFi.isPoweredOn();

* ``powerOff()`` leaves the network (and stops the access point, if any), then pulls ``WL_ON`` low.  The SSID and
  password of the last ``begin()`` are remembered; a static address set with ``config()`` is kept.  **Bluetooth stops too**: the CYW43 chip carries both WiFi and
  Bluetooth, and ``powerOff()`` powers all of it off, ending a Bluetooth link and advertising.  ``powerOn()`` restores WiFi only; a sketch that uses Bluetooth (BTstack) has to
  start the Bluetooth stack again after ``powerOn()`` (tested on a Pico 2 W with BTstack and a laptop connected, 10 of 10 cycles: the link was gone after ``powerOff()`` and
  nothing advertised again after ``powerOn()``).
* ``powerOn()`` powers the chip up again (the firmware is loaded again) and, with ``reconnect``, joins the same
  network again like ``begin()`` does, blocking until connected or ``timeoutMs`` (twice that when ``begin()`` itself blocks) have passed, also in a sketch that called ``WiFi.mode()``.  With
  ``reconnect = false`` the chip is only powered up (and left in station mode); call ``begin()`` later.
* The three calls exist on every board so that a sketch builds everywhere; on a board without the CYW43 chip ``powerOff()`` and ``powerOn()`` do nothing and return ``false``, and ``isPoweredOn()`` is ``false``.
* Sockets and server objects do not survive: re-create them after ``powerOn()``.
* Timing, Pico 2 W: the chip is up and joined after about 6.8 s (firmware load about 0.4 s, the rest is scan,
  association and DHCP).  A remembered channel and BSSID, or a static address, did not shorten it measurably, so none is kept.
  Some reconnects take 10 s or more (up to 18 s seen), when the first join attempt fails and the driver retries.
  How long it takes depends on the access point: 6 to 15 s were measured on one, so the default ``timeoutMs`` of 15 s can be too short for a slow one (``powerOn()`` then returns false while the join may still
  complete a little later); pass a larger ``timeoutMs`` for such a network.
  The reconnect costs about 0.43 C (62 mA for 7 s), so powering off pays when the sleep is longer than about 9 s; for
  shorter ones keep the station connected (``LowPower`` light sleep keeps a connected station alive).
* Pico 2 W board current at 3.7 V: connected and in ``LowPower.sleepFor()`` about 51 mA, chip powered off about 4.4 mA
  (the RP2350's own sleep floor; 3.6 mA in ``dormantFor``).  See the ``WiFiPowerOffSleep`` example.

Important Information
---------------------

* Adding WiFi increases flash usage by over 220KB

  * There is a 220KB binary firmware blob for the WiFi chip (CYW43-series) which the Pico W uses, even to control the onboard LED.

* Adding WiFi increases RAM usage by ~40KB.

  * LWIP, the TCP/IP driver, requires preallocated buffers to allow it to run in non-polling mode (i.e. packets can be sent and received in the background without the application needing to explicitly do anything).

* The WiFi driver has some limitations stemming from the upstream SDK:

  * Extensible Authentication Protocol (EAP) is not supported

  * Combined STA/AP mode is not supported

* Multicore is supported, but only core 0 may run ``WiFi`` related code when in bare metal.  When using FreeRTOS, any Task on any core can perform networking operations.


The WiFi library borrows much work from the `ESP8266 Arduino Core <https://github.com/esp8266/Arduino>`__ , especially the ``WiFiClient`` and ``WiFiServer`` classes.

Special Thanks
--------------

Special thanks to:

* @todbot for donating one of his Pico W boards to the effort

* @d-a-v for much patient explanation about LWIP internals

* The whole ESP8266 Arduino team for their network classes

* Adafruit Industries for their kind donation
