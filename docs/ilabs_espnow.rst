iLabs ESP-NOW
=============

ESP-NOW is a connectionless 2.4GHz protocol from Espressif.  It lets small
packets travel directly between devices without a WiFi access point and
without any TCP/IP stack.  This is useful for sensor networks, remote
controls and other cases where you only need to move a few bytes now and
then with low latency.

The RP2040 and RP2350 do not have a radio of their own.  On the iLabs
Challenger WiFi boards the radio is a separate ESP32-C6 or ESP32-C3
co-processor sitting next to the host processor.  This library gives you the
Arduino ESP-NOW API on the host and talks to the co-processor over a UART
link, using the iLabs ``AT+EN`` command set.  The co-processor must be flashed
with the iLabs ESP-NOW interpreter firmware for this to work.

The programming surface is the same ``ESP_NOW`` object and ``ESP_NOW_Peer``
class as in the arduino-esp32 core.  A sketch written for an ESP32 therefore
builds against this library with only the radio bring-up changed, as described
below.

::

     RP2040 / RP2350 host              ESP32-C6 co-processor
    (this Arduino library)            (AT+EN interpreter)
          |                                   |
          |   UART  (ESP_SERIAL_PORT)         |
          | ESP_NOW.begin()  --> AT+ENINIT -->|             ESP-NOW
          | peer.send()      --> AT+ENSEND -->| ))) 2.4GHz ((( peers
          | ESP_NOW.poll()   <-- +ENRECV -----|

Supported boards
~~~~~~~~~~~~~~~~~

ESP-NOW is available on the iLabs boards that carry an ESP32 co-processor:

* Challenger 2040 WiFi
* Challenger 2040 WiFi/BLE
* Challenger 2040 WiFi6/BLE
* Challenger NB 2040 WiFi
* Connectivity 2040 LTE/WiFi/BLE
* RPICO32
* Challenger 2350 WiFi6/BLE5

On any other board the library will not build.  The reason is that it needs a
board variant that wires up the ESP32 UART, and that is only true for the
boards above.

Selecting the ESP-NOW firmware type
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Pick the board in the IDE, open the Tools menu and set **ESP Wifi Type** to
**ESP-NOW**.  This defines ``ILABS_ESPNOW`` for the build.  The default value
is **ESP AT**, so a board keeps its normal AT firmware behaviour until you
change the menu.  On the two boards that also offer **ESP Hosted**, ESP-NOW is
simply added as a third choice.

When ESP-NOW is selected the board variant no longer resets the ESP32 during
start-up.  The library takes over the reset sequence instead, so that it can
wait for the co-processor to report that it is ready.  See `Board reset
handling`_ below.

If you build with PlatformIO the Tools menu is not available.  Add the define
yourself in ``platformio.ini``::

    build_flags = -DILABS_ESPNOW

Differences from an ESP32 sketch
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Because the radio is a co-processor over a UART, and not a radio inside the
main chip, two things change compared to a normal ESP32 ESP-NOW sketch.

First, the radio bring-up.  On an ESP32 you would start WiFi and set the
channel.  Here you call one function instead.  The library already knows which
UART the board variant uses, so the sketch never names a serial port:

.. code:: cpp

    ESP_NOW.setLink(channel);   // opens the ESP32 UART and resets the co-processor

Second, the servicing of incoming packets.  Call ``ESP_NOW.poll()`` once every
time through ``loop()``, in the same way you would call ``ArduinoOTA.handle()``.
Received frames are handed to the peer callbacks from inside ``poll()``.  Keep
``loop()`` free of long blocking calls.  On the RP2040 a plain ``delay()`` does
not yield, so a long ``delay()`` will hold up the receive path.  For work that
must happen at a fixed interval, use a timer to raise a flag and do the sending
from ``loop()`` when the flag is set.

Everything else stays the same as in the arduino-esp32 API.  Peer subclassing,
``begin()``, ``add()``, ``send()``, ``onReceive()``, ``onSent()``,
``onNewPeer()``, broadcast peers and ``ESP_NOW.write()`` all behave as before.

A first sketch
~~~~~~~~~~~~~~

This is a small unicast example.  It sends a short message once a second and
prints whatever it receives.  Set the peer MAC address to the address of the
other board.

.. code:: cpp

    #include <Arduino.h>
    #include "ESP32_NOW.h"

    class Peer : public ESP_NOW_Peer {
    public:
        Peer(const uint8_t *mac) : ESP_NOW_Peer(mac, 6, WIFI_IF_STA, nullptr) {}
        void onReceive(const uint8_t *data, size_t len, bool broadcast) {
            Serial.write(data, len);
            Serial.println();
        }
        using ESP_NOW_Peer::add;
        using ESP_NOW_Peer::send;
    };

    uint8_t peerMac[6] = { 0xF0, 0xF5, 0xBD, 0x31, 0x9B, 0xB0 };
    Peer peer(peerMac);

    void setup() {
        Serial.begin(115200);
        ESP_NOW.setLink(6);   // channel 6, uses the board variant's ESP32 UART
        ESP_NOW.begin();
        peer.add();
    }

    void loop() {
        peer.send((const uint8_t *)"hello", 5);
        for (uint32_t t = millis(); millis() - t < 1000;) {
            ESP_NOW.poll();
            delay(5);
        }
    }

More examples are installed with the library under **File, Examples**.  They
include the two canonical arduino-esp32 examples, **Broadcast_Master** and
**Broadcast_Slave**, a **Unicast_PingPong**, a **Discovery_PingPong** that
finds the other board by itself, and a **RegressionSuite** that runs a
two-board self test of the whole API.

Board reset handling
~~~~~~~~~~~~~~~~~~~~~

``setLink()`` opens the ESP32 UART.  When the board variant also describes the
ESP32 reset pins, which all the iLabs Challenger WiFi and WiFi6 boards do, the
library performs a hardware reset of the co-processor into run mode and waits
for its ``+ENREADY`` message before it returns.  Every time the host starts,
by power-on or by reset, the co-processor is therefore given a clean cold
start with no peers, keys or baud rate left over from an earlier session.

If the co-processor restarts on its own while running, for example after a
brownout, it sends ``+ENREADY`` again and the library records it.  You can
react to this by registering a handler, or by polling ``wasReset()`` from
``loop()``:

.. code:: cpp

    ESP_NOW.onReset([](void *) {
        // The co-processor lost its peers and keys. The simplest recovery
        // is a full restart of the host.
        rp2040.reboot();
    }, nullptr);

Discovery
~~~~~~~~~

``ESP_NOW.discover()`` is an iLabs addition and has no arduino-esp32
counterpart.  It sends a broadcast probe, and every board that runs the
interpreter answers in firmware without any help from its host.  This is a
simple way to find the other boards on the current channel without hard-coding
MAC addresses:

.. code:: cpp

    ESP_NOW_Found found[8];
    int n = ESP_NOW.discover(found, 8, 1000);   // collect answers for 1 second
    for (int i = 0; i < n; i++) {
        // found[i].mac   the responder STA MAC address
        // found[i].rssi  the signal strength this board measured
    }

The call blocks for the collection window and returns the number of boards
found, or -1 on error.  It only covers the channel that is currently in use.

Notes and limitations
~~~~~~~~~~~~~~~~~~~~~~

* The callbacks run in the context of whoever calls ``poll()``, not in a
  background task.  Keep them short, as you would on the ESP32.
* You may call library methods such as ``peer.send()`` or ``add()`` from
  inside a callback.  The library defers such calls and runs them in order
  once the callback returns.  For that reason a command issued from a callback
  returns before it has actually run, so do not issue a query from a callback
  and expect its answer straight away.
* ``getMaxDataLen()`` reports 250 bytes, which is the ESP-NOW version 1 limit.
  A unicast send above 248 bytes is fragmented by the firmware for you.
* Per-peer PHY rate, RSSI and statistics accessors, and ESP-NOW version 2
  framing are not wrapped yet.
