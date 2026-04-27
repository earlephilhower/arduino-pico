Ethernet over USB
=================

A USB connection with a PC or Smartphone can be used as a wired network connection. It requires no additional hardware.
Other USB functions like ``Serial``, ``Keyboard``, ``Mouse`` or ``Joystick`` can be used at the same time.
Both IPv4 and IPv6 are supported.

The protocol is called Network Control Model (NCM) and is the newest of three possible protocols defined by the USB standard.
NCM is natively supported by Windows, Linux, macOS, Android, ChromeOS, iOS and more.

To use it, both the Raspberry Pi Pico and the USB Host need to be configured.

USB Device configuration on Raspberry Pi Pico
---------------------------------------------

Add this to your sketch:

.. code:: cpp

    #include <NCMEthernetlwIP.h>
    

And add a global Ethernet object of the same type:

.. code:: cpp

    NCMEthernetlwIP eth;

In your ``setup()``, add this:

.. code:: cpp

    void setup() {
        eth.begin();
        ....
    }

You can use ``eth.connected()`` to check the state of the connection.

.. code:: cpp

    void setup() {
        ....
        eth.begin();

        while (!eth.connected()) {
            Serial.print(".");
        }

        Serial.print("IP address: ");
        Serial.println(eth.localIP());

        ....
    }

The Raspberry Pi Pico will try to get an IP Address via DHCP and stateless DHCPv6.
Alternatively, static addresses may be set:

.. code:: cpp

    IPAddress my_static_ip_addr(192, 168, 137, 100);
    IPAddress my_static_gateway(192, 168, 137, 1);
    IPAddress my_static_dns(192, 168, 137, 1);

    void setup() {
        ....
        eth.config(my_static_ip_addr, my_static_gateway, IPAddress(255, 255, 255, 0), my_static_dns);
        eth.begin();
        ....
    }

See also the examples:

* ``lwIP_USB_NCM/WiFiClient-NCMEthernet``

* ``lwIP_USB_NCM/WiFiClient-NCMEthernet-platformio``


USB Host configuration on Windows
---------------------------------

The Raspberry Pi Pico will appear as a USB to Ethernet adapter, creating an additional network interface on the host.
It must be configured to allow a successful network connection.

On Windows we must use `Internet Connection Sharing <https://en.wikipedia.org/wiki/Internet_Connection_Sharing>`__.
It provides DHCP on the 192.168.137.0/24 subnet and routing to the rest of the network using network address translation.

1. Win + R, then type ``ncpa.cpl``
2. Connect Raspberry Pi Pico with a NCM-enabled sketch
3. A new network connection should appear
4. Right click your **normal** network connection, select *Properties*.
5. Select the *Sharing* Tab
6. Check the box for *Allow other network users to connect through this computer’s Internet connection*
7. As *Home networking connection*, choose the **new** network connection that appeared in Step 3.

The Pico may get any address in 192.168.137.0/24. Either print it on a serial terminal or use a static IP address.
It will respond to pings if everything is setup correctly.

USB Host configuration on Linux
-------------------------------

The Raspberry Pi Pico will appear as a USB to Ethernet adapter, creating an additional network interface on the host.
It must be configured to allow a successful network connection.
Network configuration on Linux depends on your network management software.
If you don't know which one you are running, try these commands:

- ``sudo systemctl status NetworkManager``
- ``sudo systemctl status systemd-networkd``
- ``sudo systemctl status dhcpcd``

One of them will be ``active (running)``.

NetworkManager
~~~~~~~~~~~~~~

1. Prepare your Raspberry Pi Pico with a NCM-enabled sketch and configure it with DHCP or a static ip address.
2. ``ip link`` to check your existing network interfaces
3. Connect the Raspberry Pi Pico
4. ``ip link`` again, a new network interface should be listed. This example assumes ``usb0``
5. ``nmcli connection add type ethernet ifname usb0 con-name pico ipv4.method shared ipv4.address 192.168.142.1/24``
6. ``sudo iptables -A FORWARD -i usb0 -j ACCEPT``
7. ``sudo iptables -A FORWARD -o usb0 -j ACCEPT``
8. ``pyserial-miniterm /dev/ttyACM0``. Check the IP address it reports. Check for successful requests. Alternatives to ``pyserial-miniterm`` are ``picocom`` or ``minicom``.
9. ``ping <ip address from previous step>``
10. make iptables rules persistent by adding them to the files in ``/etc/iptables/*.rules``


dhcpcd
~~~~~~

dhcpcd cannot act as a DHCP server, so we must use static ip addresses.

1. Prepare your Raspberry Pi Pico with a NCM-enabled sketch and configure it with static ip address ``192.168.137.100`` and gateway ``192.168.137.1``.
2. ``ip link`` to check your existing network interfaces
3. Connect the Raspberry Pi Pico
4. ``ip link`` again, a new network interface should be listed. This example assumes ``usb0``
5. add the following to ``/etc/dhcpcd.conf``:

.. code::

    interface usb0
    static ip_address=192.168.137.1/24

6. ``systemctl restart dhcpcd``
7. ``ping 192.168.137.100``
8. ``sudo sysctl -w net.ipv4.conf.all.forwarding=1``
9. ``sudo iptables -A FORWARD -i usb0 -j ACCEPT``
10. ``sudo iptables -A FORWARD -o usb0 -j ACCEPT``
11. ``pyserial-miniterm /dev/ttyACM0``. Check for successful requests. Alternatives to ``pyserial-miniterm`` are ``picocom`` or ``minicom``.
12. make iptables rules persistent by adding them to the files in ``/etc/iptables/*.rules``

systemd-networkd
~~~~~~~~~~~~~~~~

1. Prepare your Raspberry Pi Pico with a NCM-enabled sketch and configure it with DHCP or static ip address ``192.168.137.100`` and gateway ``192.168.137.1``.
2. ``ip link`` to check your existing network interfaces
3. Connect the Raspberry Pi Pico
4. ``ip link`` again, a new network interface should be listed. This example assumes ``usb0``
5. create ``/etc/systemd/network/pico.network`` with this content:

.. code::

    [Match]
    Name=usb0

    [Network]
    Address=192.168.137.1/24

    IPv4Forwarding=yes
    IPMasquerade=yes
    DHCPServer=yes

    IPv6Forwarding=yes
    IPv6SendRA=yes
    DHCPPrefixDelegation=yes

    [DHCPServer]
    EmitDNS=yes
    UplinkInterface=:auto

    [IPv6SendRA]
    EmitDNS=yes
    UplinkInterface=:auto

6. ``sudo iptables -A FORWARD -i usb0 -j ACCEPT``
7. ``sudo iptables -A FORWARD -o usb0 -j ACCEPT``
8. ``sudo systemctl restart systemd-networkd``
9. ``pyserial-miniterm /dev/ttyACM0``. Check the IP address it reports. Check for successful requests. Alternatives to ``pyserial-miniterm`` are ``picocom`` or ``minicom``.
10. ``ping <ip address from previous step>``
11. make iptables rules persistent by adding them to the files in ``/etc/iptables/*.rules``

Manual config
~~~~~~~~~~~~~

This config will not survive a Linux reboot, but may be ok for testing.

1. Prepare your Raspberry Pi Pico with a NCM-enabled sketch and configure it with static ip address ``192.168.137.100`` and gateway ``192.168.137.1``.
2. ``ip link`` to check your existing network interfaces
3. Connect the Raspberry Pi Pico
4. ``ip link`` again, a new network interface should be listed. This example assumes ``usb0``
5. You can also check ``lsusb -t`` and ``dmesg``
6. ``ip link set usb0 up``
7. ``ip addr add 192.168.137.1/24 dev usb0``
8. ``ping 192.168.137.100``

Special Thanks
--------------

* tinyUSB contributors for the NCM implementation

* NCMEthernetlwIP driver written by functionpointer

