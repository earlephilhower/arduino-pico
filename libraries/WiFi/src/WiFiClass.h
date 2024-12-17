/*
    WiFi.h - WiFi class "compat" w/WiFiNINA for Raspberry Pi Pico W
    Copyright (c) 2022 Earle F. Philhower, III.  All rights reserved.

    Implements the API defined by the Arduino WiFiNINA library,
    copyright (c) 2018 Arduino SA. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <Arduino.h>
#if defined(PICO_CYW43_SUPPORTED)
#include <lwIP_CYW43.h>
#elif defined(ESPHOSTSPI)
#include <lwIP_ESPHost.h>
#elif defined(WINC1501_SPI)
#include <lwIP_WINC1500.h>
#else
#include "utility/lwIP_nodriver.h"
#endif
#include "WiFi.h"

#include <inttypes.h>
#include <map>

#include "dhcpserver/dhcpserver.h"

#define WIFI_FIRMWARE_LATEST_VERSION PICO_SDK_VERSION_STRING

typedef void(*FeedHostProcessorWatchdogFuncPointer)();

typedef enum { WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3 } WiFiMode_t; // For ESP8266 compatibility

class WiFiClass {
public:
    WiFiClass();

    /*
        Get firmware version
    */
    static const char* firmwareVersion();

    void mode(WiFiMode_t m); // For ESP8266 compatibility

    WiFiMode_t getMode() {
        if (_wifiHWInitted) {
            if (_apMode) {
                return WiFiMode_t::WIFI_AP;
            } else {
                return WiFiMode_t::WIFI_STA;
            }
        }
        return WiFiMode_t::WIFI_OFF;
    };

    /*  Start WiFi connection for OPEN networks

        param ssid: Pointer to the SSID string.
    */
    int begin(const char* ssid);
    /*  Start WiFi connection for OPEN networks, without blocking

        param ssid: Pointer to the SSID string.
    */
    int beginNoBlock(const char* ssid);

    int beginBSSID(const char* ssid, const uint8_t *bssid);

    /*  Start WiFi connection with WEP encryption.
        Configure a key into the device. The key type (WEP-40, WEP-104)
        is determined by the size of the key (5 bytes for WEP-40, 13 bytes for WEP-104).

        param ssid: Pointer to the SSID string.
        param key_idx: The key index to set. Valid values are 0-3.
        param key: Key input buffer.
    */
    // TODO - WEP is not supported in the driver
    // int begin(const char* ssid, uint8_t key_idx, const char* key);

    /*  Start WiFi connection with passphrase
        the most secure supported mode will be automatically selected

        param ssid: Pointer to the SSID string.
        param passphrase: Passphrase. Valid characters in a passphrase
              must be between ASCII 32-126 (decimal).
        param bssid: If non-null, the BSSID associated w/the SSID to connect to
    */
    int begin(const char* ssid, const char *passphrase, const uint8_t *bssid = nullptr);
    /*  Start WiFi connection with passphrase, without blocking. Check for .connected() for a connection
        the most secure supported mode will be automatically selected

        param ssid: Pointer to the SSID string.
        param passphrase: Passphrase. Valid characters in a passphrase
              must be between ASCII 32-126 (decimal).
        param bssid: If non-null, the BSSID associated w/the SSID to connect to
    */
    int beginNoBlock(const char* ssid, const char *passphrase, const uint8_t *bssid = nullptr);

    bool connected();
    bool isConnected() {
        return connected();
    }
    int8_t waitForConnectResult(unsigned long timeoutLength = 60000) {
        uint32_t now = millis();
        while (millis() - now < timeoutLength) {
            if (status() != WL_DISCONNECTED) {
                return status();
            }
            delay(10);
        }
        return -1;
    }

    uint8_t beginAP(const char *ssid);
    uint8_t beginAP(const char *ssid, uint8_t channel);
    uint8_t beginAP(const char *ssid, const char* passphrase);
    uint8_t beginAP(const char *ssid, const char* passphrase, uint8_t channel);

    // ESP8266 compatible calls
    bool softAP(const char* ssid, const char* psk = nullptr, int channel = 1, int ssid_hidden = 0, int max_connection = 4) {
        (void) ssid_hidden;
        (void) max_connection;
        return beginAP(ssid, psk, channel) == WL_CONNECTED;
    }

    bool softAP(const String& ssid, const String& psk = "", int channel = 1, int ssid_hidden = 0, int max_connection = 4) {
        (void) ssid_hidden;
        (void) max_connection;
        if (psk != "") {
            return beginAP(ssid.c_str(), psk.c_str(), channel) == WL_CONNECTED;
        } else {
            return beginAP(ssid.c_str(), channel) == WL_CONNECTED;
        }
    }

    bool softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet) {
        config(local_ip, local_ip, gateway, subnet);
        return true;
    }

    bool softAPdisconnect(bool wifioff = false) {
        (void) wifioff;
        disconnect();
        return true;
    }

#if defined(PICO_CYW43_SUPPORTED)
    uint8_t softAPgetStationNum();
#endif

    IPAddress softAPIP() {
        return localIP();
    }

    uint8_t* softAPmacAddress(uint8_t* mac) {
        return macAddress(mac);
    }

    String softAPmacAddress(void) {
        uint8_t mac[6];
        macAddress(mac);
        char buff[32];
        sprintf(buff, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return String(buff);
    }

    String softAPSSID() {
        return SSID();
    }


    // TODO - EAP is not supported by the driver.  Maybe some way of user-level wap-supplicant in the future?
    //uint8_t beginEnterprise(const char* ssid, const char* username, const char* password);
    //uint8_t beginEnterprise(const char* ssid, const char* username, const char* password, const char* identity);
    //uint8_t beginEnterprise(const char* ssid, const char* username, const char* password, const char* identity, const char* ca);

    /*  Change Ip configuration settings disabling the dhcp client

          param local_ip: 	Static ip configuration
    */
    void config(IPAddress local_ip);

    /*  Change Ip configuration settings disabling the dhcp client

          param local_ip: 	Static ip configuration
        param dns_server:     IP configuration for DNS server 1
    */
    void config(IPAddress local_ip, IPAddress dns_server);

    /*  Change Ip configuration settings disabling the dhcp client

          param local_ip: 	Static ip configuration
        param dns_server:     IP configuration for DNS server 1
          param gateway : 	Static gateway configuration
    */
    void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway);

    /*  Change Ip configuration settings disabling the dhcp client

          param local_ip: 	Static ip configuration
        param dns_server:     IP configuration for DNS server 1
          param gateway: 	Static gateway configuration
          param subnet:		Static Subnet mask
    */
    void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);

    /*  Change DNS Ip configuration

        param dns_server1: ip configuration for DNS server 1
    */
    void setDNS(IPAddress dns_server1);

    /*  Change DNS Ip configuration

        param dns_server1: ip configuration for DNS server 1
        param dns_server2: ip configuration for DNS server 2

    */
    void setDNS(IPAddress dns_server1, IPAddress dns_server2);


    /*  Set the hostname used for DHCP requests

        param name: hostname to set

    */
    void setHostname(const char* name);
    const char *getHostname();

    /*
        Disconnect from the network

        return: one value of wl_status_t enum
    */
    int disconnect(bool wifi_off = false);

    void end(void);

    /*
        Get the interface MAC address.

        return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
    */
    uint8_t* macAddress(uint8_t* mac);
    String macAddress(void) {
        uint8_t mac[6];
        macAddress(mac);
        char buff[32];
        sprintf(buff, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return String(buff);
    }

    /*
        Get the interface IP address.

        return: Ip address value
    */
    IPAddress localIP();

    /*
        Get the interface subnet mask address.

        return: subnet mask address value
    */
    IPAddress subnetMask();

    /*
        Get the gateway ip address.

        return: gateway ip address value
    */
    IPAddress gatewayIP();

    /*
        Get the DNS ip address.

        return: IPAddress DNS Server IP
    */
    IPAddress dnsIP(uint8_t dns_no = 0);

    /*
        Return the current SSID associated with the network

        return: ssid string
    */
    const String &SSID();

    /*
        Return the current BSSID associated with the network.
        It is the MAC address of the Access Point

        return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
    */
    uint8_t* BSSID(uint8_t* bssid);

    /*
        Return the current RSSI /Received Signal Strength in dBm)
        associated with the network

        return: signed value
    */
    int32_t RSSI();


    /* Return the current network channel */
    int channel();

    /*
        Return the Encryption Type associated with the network

        return: one value of wl_enc_type enum
    */
    uint8_t	encryptionType();

    /*
        Start scan WiFi networks available

        param async: whether to perform asynchronous scan

        return: Number of discovered networks
    */
    int8_t scanNetworks(bool async = false);

    /*
        Return number of scanned WiFi networks

        return: Number of discovered networks
    */
    int8_t scanComplete();

    /*
        Delete scan results
    */
    void scanDelete();

    /*
        Return the SSID discovered during the network scan.

        param networkItem: specify from which network item want to get the information

        return: ssid string of the specified item on the networks scanned list
    */
    const char*	SSID(uint8_t networkItem);

    /*
        Return the encryption type of the networks discovered during the scanNetworks

        param networkItem: specify from which network item want to get the information

        return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
    */
    uint8_t	encryptionType(uint8_t networkItem);

    uint8_t* BSSID(uint8_t networkItem, uint8_t* bssid);
    uint8_t channel(uint8_t networkItem);

    /*
        Return the RSSI of the networks discovered during the scanNetworks

        param networkItem: specify from which network item want to get the information

        return: signed value of RSSI of the specified item on the networks scanned list
    */
    int32_t RSSI(uint8_t networkItem);

    /*
        Return Connection status.

        return: one of the value defined in wl_status_t
    */
    uint8_t status();

    /*
        Return The deauthentication reason code.

        return: the deauthentication reason code
    */
    uint8_t reasonCode();

    /*
        Resolve the given hostname to an IP address.
        param aHostname: Name to be resolved
        param aResult: IPAddress structure to store the returned IP address
        result: 1 if aIPAddrString was successfully converted to an IP address,
                else error code
    */
    int hostByName(const char* aHostname, IPAddress& aResult) {
        return hostByName(aHostname, aResult, _timeout);
    }
    int hostByName(const char* aHostname, IPAddress& aResult, int timeout);

    unsigned long getTime();

#if defined(PICO_CYW43_SUPPORTED)
    void aggressiveLowPowerMode();
    void defaultLowPowerMode();
#endif
    void noLowPowerMode();

    int ping(const char* hostname, uint8_t ttl = 128);
    int ping(const String &hostname, uint8_t ttl = 128);
    int ping(IPAddress host, uint8_t ttl = 128);

    void setTimeout(unsigned long timeout);

    void setFeedWatchdogFunc(FeedHostProcessorWatchdogFuncPointer func);
    void feedWatchdog();

    // ESP8266 compatibility
    void persistent(bool unused) {
        (void) unused;
    }

    void hostname(const char *name) {
        setHostname(name);
    }

private:
    // Internal wifi begin. Returns true on success
    bool _beginInternal(const char* ssid, const char *passphrase, const uint8_t *bssid = nullptr);

    int _timeout = 15000;
    String _ssid;
    uint8_t _bssid[6];
    String _password;
    bool _wifiHWInitted = false;
    bool _apMode = false;

    // DHCP for AP mode
    dhcp_server_t *_dhcpServer = nullptr;

    // ESP compat
    bool _calledESP = false; // Should we behave like the ESP8266 for connect?
    WiFiMode_t _modeESP = WIFI_STA;
};

extern WiFiClass WiFi;
