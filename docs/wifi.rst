WiFi (Raspberry Pi Pico W) Support
==================================

WiFi is supported on the Raspberry Pi Pico W by selecting the "Raspberry Pi Pico W" board in the Boards Manager.  It is generally compatible with the `Arduino WiFi library <https://www.arduino.cc/en/Reference/WiFi>`__ and the `ESP8266 Arduino WiFi library <https://github.com/esp8266/Arduino>`__.

Enable WiFi support by selecting the `Raspberry Pi Pico W` board in the IDE and adding ``#include <WiFi.h>`` in your sketch.

Supported Features
------------------

* WiFi connection (Open, WPA/WPA2) 

  * Static IP or dynamic DHCP supported

  * Station Mode (STA, connects to an existing network)

  * Access Point Mode (AP, creates its own wireless network) with 4 clients

* WiFi Scanning and Reporting
 
  * See the ``ScanNetworks.ino`` example to better understand the process.


Important Information
---------------------

Please note that WiFi on the Pico W is a work-in-progress and there are some important caveats:

* Adding WiFi increases flash usage by over 220KB

  * There is a 220KB binary firmware blob for the WiFi chip (CYW43-series) which the Pico W uses, even to control the onboard LED.

* Adding WiFi increases RAM usage by ~40KB.

  * LWIP, the TCP/IP driver, requires preallocated buffers to allow it to run in non-polling mode (i.e. packets can be sent and received in the background without the application needing to explicitly do anything).

* The WiFi driver is a little limited as of now, but fully functional for sending and receiving data

  * Extensible Authentication Protocol (EAP) is not supported

  * Combined STA/AP mode is not supported

  * Certain WiFi status values (RSSI, BSSID, etc.) are not available.

* Multicore is supported, but only one core may run ``WiFi`` code.

  * FreeRTOS is not yet supported due to the requirement for a very different LWIP implementation.  PRs always appreciated!

The WiFi library borrows much work from the `ESP8266 Arduino Core <https://github.com/esp8266/Arduino>`__ , especially the ``WiFiClient`` and ``WiFiServer`` classes.

Special Thanks
--------------

Special thanks to:

* @todbot for donating one of his Pico W boards to the effort

* @d-a-v for much patient explanation about LWIP internals

* The whole ESP8266 Arduino team for their network classes

* Adafruit Industries for their kind donation
