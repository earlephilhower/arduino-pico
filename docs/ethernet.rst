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

* Wiznet W5100(s)

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

Adjusting LWIP Polling
----------------------

LWIP operates in a polling mode for the wired Ethernet devices.  By default it will run
every 20ms, meaning that on average it will take half that time (10ms) before a packet
received in the Ethernet module is received and operated upon by the Pico.  This gives
very low CPU utilization but in some cases this latency can affect performance.

Adding a call to ``lwipPollingPeriod(XXX)`` (where ``XXXX`` is the polling period in
milliseconds) can adjust this setting on the fly.  Note that if you set it too low, the
Pico may not have enough time to service the Ethernet port before the timer fires again,
leading to a lock up and hang.


Using Interrupt-Driven Handling
-------------------------------

The WizNet and ENC28J60 devices support generating an interrupt when a packet is received,
removing the need for polling and decreasing latency.  Simply specify the SPI object to use and the
interrupt pin when instantiating the Ethernet object:

.. code:: cpp

    #include <W5100lwIP.h>
    Wiznet5100lwIP eth(SS /* Chip Select*/, SPI /* SPI interface */, 17 /* Interrupt GPIO */ );


Adjusting SPI Speed
-------------------

By default a 4MHz clock will be used to clock data into and out of the Ethernet module.
Depending on the module and your wiring, a higher SPI clock may increase performance (but
too high of a clock will cause communications problems or hangs).

This value may be adjusted using the ``eth.setSPISpeed(hz)`` call **before** starting the
device.  (You may also use custom ``SPISettings`` instead via ``eth.setSPISettings(spis)```)

For example, to set the W5500 to use a 30MHZ clock:

.. code:: cpp

    #include <W5500lwIP.h>
    Wiznet5500lwIP eth(1);

    void setup() {
        eth.setSPISpeed(30000000);
        lwipPollingPeriod(3);
        ...
        eth.begin();
        ...
    }

Using the WIZnet W5100S-EVB-Pico
--------------------------------

You can use the onboard Ethernet chip with these drivers, in interrupt mode, by utilizing the following options:

.. code:: cpp

    #include <W5100lwIP.h>
    Wiznet5100lwIP eth(17, SPI, 21);  // Note chip select is **17**

    void setup() {
        // Set SPI to the onboard Wiznet chip
        SPI.setRX(16);
        SPI.setCS(17);
        SPI.setSCK(18);
        SPI.setTX(19);
        ...
        eth.begin();
        ...
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

