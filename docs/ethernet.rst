EthernetLWIP (Wired Ethernet) Support
=====================================

Wired Ethernet interfaces are supported for all the internal networking
libraries (``WiFiClient``, ``WiFiClientSecure``, ``WiFiServer``,
``WiFiServerSecure``, ``WiFiUDP``, ``WebServer``, ``Updater``,
``HTTPClient``, etc.).

Using these wired interfaces is very similar to using the Pico-W WiFi
so most examples in the core only require minor modifications to use
a wired interface.

Supported Wired Ethernet Modules
--------------------------------

* Wiznet W5100

* Wiznet W5500

* ENC28J60


Enabling Wired Ethernet
-----------------------

Simply replace the WiFi include at the top with:

.. code:: cpp

    #include <W5500lwIP.h> // Or W5100lwIP.h or ENC28J60.h
    

And add a global Ethernet object of the same type:

.. code:: cpp

    Wiznet5500lwIP eth(1); // Parameter is the Chip Select pin

In your ``setup()`` you may adjust the SPI pins you're using to
match your hardware (be sure they are legal for the RP2040!), or
skip this if you're using the default ones:

.. code:: cpp

    void setup() {
        SPI.setRX(0);
        SPI.setCS(1);
        SPI.setSCK(2);
        SPI.setTX(3);
        ....
    }

And finally replace the ``WiFi.begin()`` and ``WiFi.connected()``
calls with ``eth.begin()`` and ``eth.connected()``:

.. code:: cpp

    void setup() {
        ....
        // WiFi.begin(SSID, PASS)
        eth.begin();
        
        //while (!WiFi.connected()) {
        while (!eth.connected()) {
            Serial.print(".");
        }

        Serial.print("IP address: ");
        //Serial.println(WiFi.localIP());
        Serial.println(eth.localIP());

        ....
    }

Example Code
------------

The following examples allow switching between WiFi and Ethernet:

* ``WebServer/AdvancedWebServer``

* ``HTTPClient/BasicHTTPSClient``

Caveats
-------

The same restrictions for ``WiFi`` apply to these Ethernet classes, namely:

* Only core 0 may run any networking related code.

* In FreeRTOS, only the ``setup`` and ``loop`` task can call networking libraries, not any tasks.

Special Thanks
--------------

* LWIPEthernet classes come from the ESP8266 Arduino team

* Individual Ethernet drivers were written by Nicholas Humfrey

