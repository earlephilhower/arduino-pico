#pragma once

#include <LwipIntfDev.h>
#include <W5100lwIP.h>
#include <W5500lwIP.h>
#include <ENC28J60lwIP.h>

#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

using EthernetUDP = WiFiUDP;
using EthernetClient = WiFiClient;
using EthernetServer = WiFiServer;

enum HardwareStatus {
    EthernetNoHardware,
    EthernetHardwareFound,
};

template<class RawDev>
class ArduinoEthernet : public LwipIntfDev<RawDev> {
public:
    ArduinoEthernet(int8_t cs = SS, SPIClass &spi = SPI, int8_t intr = -1) :
        LwipIntfDev<RawDev>(cs, spi, intr) {
        _hardwareStatus = EthernetNoHardware;
    }

    // Arduino-Ethernet API compatibility, order can be either:
    // mac, ip, gateway, netmask, dns (esp8266 or natural order)
    // mac, ip, dns, gateway, netmask (Arduino legacy)
    boolean begin(const uint8_t *macAddress, IPAddress local_ip = IPADDR_NONE, IPAddress arg1 = IPADDR_NONE, IPAddress arg2 = IPADDR_NONE, IPAddress arg3 = IPADDR_NONE) {
        if (local_ip.isSet() && local_ip.isV4()) {
            // setting auto values using arduino ordering of parameters
            if (arg1 == IPADDR_NONE) {  // else dns or gw
                arg1 = local_ip;
                arg1[3] = 1;
            }
            if (arg2 == IPADDR_NONE) {  // else gw or mask
                arg2 = local_ip;
                arg2[3] = 1;
            }
            // if arg2 is mask (esp ordering), let DNS IP unconfigured
            if (arg3 == IPADDR_NONE && arg2[0] != 255) {  // else mask or dns
                arg3 = IPAddress(255, 255, 255, 0);
            }
        }
        bool ret = true;
        if (local_ip.isSet()) {
            ret = LwipIntfDev<RawDev>::config(local_ip, arg1, arg2, arg3);
        }
        if (ret) {
            ret = LwipIntfDev<RawDev>::begin(macAddress);
            if (!local_ip.isSet()) {
                // Arduino API waits for DHCP answer
                while (!LwipIntfDev<RawDev>::connected()) {
                    delay(100);
                }
            }
        }

        if (ret) {
            _hardwareStatus = EthernetHardwareFound;
        }

        return ret;
    }

    void end() {
        ip_addr_copy(LwipIntfDev<RawDev>::_netif.ip_addr, ip_addr_any);  // to allow DHCP at next begin
        LwipIntfDev<RawDev>::end();
    }

    void MACAddress(uint8_t *mac) {
        LwipIntfDev<RawDev>::macAddress(mac);
    }

    IPAddress dnsServerIP() const {
        return LwipIntfDev<RawDev>::dnsIP(0);
    }

    void setDnsServerIP(const IPAddress dnsIP) {
        LwipIntfDev<RawDev>::setDNS(dnsIP);
    }

    HardwareStatus hardwareStatus() const {
        return _hardwareStatus;
    }

    int maintain() const {
        return 0;
    }

protected:
    HardwareStatus _hardwareStatus;
};

using ArduinoWiznet5500lwIP = ArduinoEthernet<Wiznet5500>;
using ArduinoWiznet5100lwIP = ArduinoEthernet<Wiznet5100>;
using ArduinoENC28J60lwIP   = ArduinoEthernet<ENC28J60>;
