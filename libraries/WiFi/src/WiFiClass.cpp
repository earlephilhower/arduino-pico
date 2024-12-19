/*
    WiFiClass.cpp - WiFi class "compat" w/WiFiNINA for Raspberry Pi Pico W
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

#include <inttypes.h>
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include <map>
#include "WiFi.h"

// This is the real WiFi network object, we just tickle it to do our magic
#include <LwipEthernet.h>
#if defined(PICO_CYW43_SUPPORTED)
#include <pico/cyw43_arch.h>
static CYW43lwIP _wifi(1);
#elif defined(ESPHOSTSPI)
static ESPHostLwIP _wifi;
#elif defined(WINC1501_SPI)
static WINC1500LwIP _wifi;
#else
static NoDriverLwIP _wifi;
#endif

WiFiClass::WiFiClass() {
}

/*
    Get firmware version
*/
const char* WiFiClass::firmwareVersion() {
    // TODO - does not look like driver reports this now
    return PICO_SDK_VERSION_STRING;
}

void WiFiClass::mode(WiFiMode_t m) {
    _calledESP = true;
    switch (m) {
    case WiFiMode_t::WIFI_OFF:
        end();
        break;
    case WiFiMode_t::WIFI_AP:
        _modeESP = WiFiMode_t::WIFI_AP;
        break;
    case WiFiMode_t::WIFI_STA:
        _modeESP = WiFiMode_t::WIFI_STA;
        break;
    case WiFiMode_t::WIFI_AP_STA:
        _modeESP = WiFiMode_t::WIFI_STA;
        break;
    }
}

bool WiFiClass::_beginInternal(const char* ssid, const char *passphrase, const uint8_t *bssid) {
    // Simple ESP8266 compatibility hack
    if (_modeESP == WIFI_AP) {
        // When beginAP was a success, it returns WL_CONNECTED and we return true as success.
        return beginAP(ssid, passphrase) == WL_CONNECTED;
    }

    end();

    _ssid = ssid;
    _password = passphrase;
    if (bssid) {
        memcpy(_bssid, bssid, sizeof(_bssid));
    } else {
        bzero(_bssid, sizeof(_bssid));
    }
    _wifi.setSTA();
    _wifi.setSSID(_ssid.c_str());
    _wifi.setBSSID(_bssid);
    _wifi.setPassword(passphrase);
    _wifi.setTimeout(_timeout);
    _apMode = false;
    _wifiHWInitted = true;

    // Internal wifi.begin returns false when failed, therefore we return false as error
    if (!_wifi.begin()) {
        return false;
    }
    noLowPowerMode();
    // Enable CYW43 event debugging (make sure Debug Port is set)
    //cyw43_state.trace_flags = 0xffff;

    return true;
}


/*  Start WiFi connection for OPEN networks

    param ssid: Pointer to the SSID string.
*/
int WiFiClass::begin(const char* ssid) {
    return begin(ssid, nullptr);
}

/*  Start WiFi connection for OPEN networks, but non blocking.

    param ssid: Pointer to the SSID string.
*/
int WiFiClass::beginNoBlock(const char* ssid) {
    return beginNoBlock(ssid, nullptr);
}


int WiFiClass::beginBSSID(const char* ssid, const uint8_t *bssid) {
    return begin(ssid, nullptr, bssid);
}

/*  Start WiFi connection with passphrase
    the most secure supported mode will be automatically selected

    param ssid: Pointer to the SSID string.
    param passphrase: Passphrase. Valid characters in a passphrase
          must be between ASCII 32-126 (decimal).
*/
int WiFiClass::begin(const char* ssid, const char *passphrase, const uint8_t *bssid) {
    uint32_t start = millis(); // The timeout starts from network init, not network link up

    // Returns WL_IDLE_STATUS on error for compatibility.
    if (!_beginInternal(ssid, passphrase, bssid)) {
        return WL_IDLE_STATUS;
    }

    while (!_calledESP && ((millis() - start < (uint32_t)2 * _timeout)) && !connected()) {
        delay(10);
    }
    return status();
}

int WiFiClass::beginNoBlock(const char* ssid, const char *passphrase, const uint8_t *bssid) {
    // Returns WL_IDLE_STATUS on error for compatibility.
    if (!_beginInternal(ssid, passphrase, bssid)) {
        return WL_IDLE_STATUS;
    }
    return status();
}

uint8_t WiFiClass::beginAP(const char *ssid) {
    return beginAP(ssid, nullptr);
}

uint8_t WiFiClass::beginAP(const char *ssid, uint8_t channel) {
    return beginAP(ssid, nullptr, channel);
}

uint8_t WiFiClass::beginAP(const char *ssid, const char* passphrase) {
    return beginAP(ssid, passphrase, 0);
}

uint8_t WiFiClass::beginAP(const char *ssid, const char* passphrase, uint8_t channel) {
    end();

    _ssid = ssid;
    _password = passphrase;
    _wifi.setAP();
    _wifi.setSSID(_ssid.c_str());
    _wifi.setPassword(passphrase);
#if defined(PICO_CYW43_SUPPORTED)
    if (channel > 0) {
        cyw43_wifi_ap_set_channel(&cyw43_state, channel);
    }
#endif
    _wifi.setTimeout(_timeout);
    _apMode = true;
    IPAddress gw = _wifi.gatewayIP();
    if (!gw.isSet()) {
        gw = IPAddress(192, 168, 42, 1);
    }
    IPAddress mask = _wifi.subnetMask();
    if (!mask.isSet()) {
        mask = IPAddress(255, 255, 255, 0);
    }
    config(gw);
    if (!_wifi.begin()) {
        return WL_IDLE_STATUS;
    }
    noLowPowerMode();
    _dhcpServer = (dhcp_server_t *)malloc(sizeof(dhcp_server_t));
    if (!_dhcpServer) {
        // OOM
        return WL_IDLE_STATUS;
    }
    dhcp_server_init(_dhcpServer, gw, mask);

    _wifiHWInitted = true;

    return WL_CONNECTED;
}

#if defined(PICO_CYW43_SUPPORTED)
uint8_t WiFiClass::softAPgetStationNum() {
    if (!_apMode || !_wifiHWInitted) {
        return 0;
    }
    int m;
    cyw43_wifi_ap_get_max_stas(&cyw43_state, &m);
    uint8_t *macs = (uint8_t*)malloc(m * 6);
    if (!macs) {
        return 0;
    }
    cyw43_wifi_ap_get_stas(&cyw43_state, &m, macs);
    free(macs);
    return m;
}
#endif

bool WiFiClass::connected() {
    return (_apMode && _wifiHWInitted) || (_wifi.connected() && localIP().isSet());
}

/*  Change Ip configuration settings disabling the dhcp client

    param local_ip: 	Static ip configuration
*/
void WiFiClass::config(IPAddress local_ip) {
    _wifi.config(local_ip);
}

/*  Change Ip configuration settings disabling the dhcp client

    param local_ip: 	Static ip configuration
    param dns_server:     IP configuration for DNS server 1
*/
void WiFiClass::config(IPAddress local_ip, IPAddress dns_server) {
    _wifi.config(local_ip, dns_server);
}

/*  Change Ip configuration settings disabling the dhcp client

    param local_ip: 	Static ip configuration
    param dns_server:     IP configuration for DNS server 1
    param gateway : 	Static gateway configuration
*/
void WiFiClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway) {
    _wifi.config(local_ip, dns_server, gateway);
}

/*  Change Ip configuration settings disabling the dhcp client

    param local_ip: 	Static ip configuration
    param dns_server:     IP configuration for DNS server 1
    param gateway: 	Static gateway configuration
    param subnet:		Static Subnet mask
*/
void WiFiClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet) {
    _wifi.config(local_ip, gateway, subnet, dns_server);
}

/*  Change DNS Ip configuration

    param dns_server1: ip configuration for DNS server 1
*/
void WiFiClass::setDNS(IPAddress dns_server1) {
    dns_setserver(0, dns_server1);
}

/*  Change DNS Ip configuration

    param dns_server1: ip configuration for DNS server 1
    param dns_server2: ip configuration for DNS server 2

*/
void WiFiClass::setDNS(IPAddress dns_server1, IPAddress dns_server2) {
    dns_setserver(0, dns_server1);
    dns_setserver(1, dns_server2);
}

/*  Set the hostname used for DHCP requests

    param name: hostname to set

*/
void WiFiClass::setHostname(const char* name) {
    _wifi.setHostname(name);
}
const char *WiFiClass::getHostname() {
    return _wifi.getHostname();
}

/*
    Disconnect from the network

    return: one value of wl_status_t enum
*/
int WiFiClass::disconnect(bool wifi_off __unused) {
    if (_dhcpServer) {
        dhcp_server_deinit(_dhcpServer);
        free(_dhcpServer);
        _dhcpServer = nullptr;
    }
    if (_wifiHWInitted) {
        _wifiHWInitted = false;
        _wifi.end();
    }
    return WL_DISCONNECTED;
}

void WiFiClass::end(void) {
    if (_wifiHWInitted) {
        disconnect();
    }
}

/*
    Get the interface MAC address.

    return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
*/
uint8_t* WiFiClass::macAddress(uint8_t* mac) {
#if defined(PICO_CYW43_SUPPORTED)
    if (!_wifiHWInitted) {
        _apMode = false;
        cyw43_wifi_set_up(&cyw43_state, _apMode ? 1 : 0, true, CYW43_COUNTRY_WORLDWIDE);
    }
#endif
    return _wifi.macAddress(_apMode, mac);
}

/*
    Get the interface IP address.

    return: Ip address value
*/
IPAddress WiFiClass::localIP() {
    return _wifi.localIP();
}

/*
    Get the interface subnet mask address.

    return: subnet mask address value
*/
IPAddress WiFiClass::subnetMask() {
    return _wifi.subnetMask();
}

/*
    Get the gateway ip address.

    return: gateway ip address value
*/
IPAddress WiFiClass::gatewayIP() {
    return _wifi.gatewayIP();
}

/*
    Get the DNS ip address.

    return: IPAddress DNS Server IP
*/
IPAddress WiFiClass::dnsIP(uint8_t dns_no) {
    return _wifi.dnsIP(dns_no);
}

/*
    Return the current SSID associated with the network

    return: ssid string
*/
const String &WiFiClass::SSID() {
    return _ssid;
}

/*
    Return the current BSSID associated with the network.
    It is the MAC address of the Access Point

    return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
*/
uint8_t* WiFiClass::BSSID(uint8_t* bssid) {
    return _wifi.BSSID(bssid);
}

int WiFiClass::channel() {
    return _wifi.channel();
}


/*
    Return the current RSSI /Received Signal Strength in dBm)
    associated with the network

    return: signed value
*/
int32_t WiFiClass::RSSI() {
    return _wifi.RSSI();
}

/*
    Return the Encryption Type associated with the network

    return: one value of wl_enc_type enum
*/
uint8_t WiFiClass::encryptionType() {
    if (_password == nullptr) {
        return ENC_TYPE_NONE;
    }
    return _wifi.encryptionType();
}

/*
    Start scan WiFi networks available

    return: Number of discovered networks
*/
int8_t WiFiClass::scanNetworks(bool async) {
    if (!_wifiHWInitted) {
        _apMode = false;
#if defined(PICO_CYW43_SUPPORTED)
        cyw43_arch_enable_sta_mode();
#endif
        _wifiHWInitted = true;
    }
    return _wifi.scanNetworks(async);
}

int8_t WiFiClass::scanComplete() {
    return _wifi.scanComplete();
}

void WiFiClass::scanDelete() {
    _wifi.scanDelete();
}

/*
    Return the SSID discovered during the network scan.

    param networkItem: specify from which network item want to get the information

    return: ssid string of the specified item on the networks scanned list
*/
const char*WiFiClass::SSID(uint8_t networkItem) {
    return _wifi.SSID(networkItem);
}

/*
    Return the encryption type of the networks discovered during the scanNetworks

    param networkItem: specify from which network item want to get the information

    return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
*/
uint8_t WiFiClass::encryptionType(uint8_t networkItem) {
    return _wifi.encryptionType(networkItem);
}

uint8_t* WiFiClass::BSSID(uint8_t networkItem, uint8_t* bssid) {
    return _wifi.BSSID(networkItem, bssid);
}

uint8_t WiFiClass::channel(uint8_t networkItem) {
    return _wifi.channel(networkItem);
}

/*
    Return the RSSI of the networks discovered during the scanNetworks

    param networkItem: specify from which network item want to get the information

    return: signed value of RSSI of the specified item on the networks scanned list
*/
int32_t WiFiClass::RSSI(uint8_t networkItem) {
    return _wifi.RSSI(networkItem);
}

/*
    Return Connection status.

    return: one of the value defined in wl_status_t
*/
uint8_t WiFiClass::status() {
    if (_apMode && _wifiHWInitted) {
        return WL_CONNECTED;
    }
    return _wifi.status();
}

/*
    Return The deauthentication reason code.

    return: the deauthentication reason code
*/
uint8_t WiFiClass::reasonCode() {
    // TODO - driver does not report this?!
    return WL_NO_SHIELD;
}

/**
    Resolve the given hostname to an IP address.
    @param aHostname     Name to be resolved
    @param aResult       IPAddress structure to store the returned IP address
    @return 1 if aIPAddrString was successfully converted to an IP address,
            else 0
*/
// Note that there is now a global FCN for name lookup to use all Ethernet ports, no need to call WiFi.hostByName, just ::hostByName
int WiFiClass::hostByName(const char* aHostname, IPAddress& aResult, int timeout_ms) {
    return ::hostByName(aHostname, aResult, timeout_ms);
}

// TODO
unsigned long WiFiClass::getTime() {
    return millis();
}

#if defined(PICO_CYW43_SUPPORTED)
void WiFiClass::aggressiveLowPowerMode() {
    cyw43_wifi_pm(&cyw43_state, CYW43_AGGRESSIVE_PM);
}

void WiFiClass::defaultLowPowerMode() {
    cyw43_wifi_pm(&cyw43_state, CYW43_DEFAULT_PM);
}
#endif

// The difference between the default CYW43_DEFAULT_PM (0xA11142) and not low power (0xA11140) is that it changed from "Powersave mode on specified interface with High throughput" to "No Powersave mode". All other parameters stayed the same.
void WiFiClass::noLowPowerMode() {
    _wifi.noLowPowerMode();
}

int WiFiClass::ping(const char* hostname, uint8_t ttl) {
    IPAddress ip;
    if (!hostByName(hostname, ip)) {
        return WL_PING_UNKNOWN_HOST;
    }
    return ping(ip, ttl);
}

int WiFiClass::ping(const String &hostname, uint8_t ttl) {
    return ping(hostname.c_str(), ttl);
}

int WiFiClass::ping(IPAddress host, uint8_t ttl) {
    return _wifi.ping(host, ttl, _timeout);
}

void WiFiClass::setTimeout(unsigned long timeout) {
    _timeout = timeout;
}

void WiFiClass::setFeedWatchdogFunc(FeedHostProcessorWatchdogFuncPointer func) {
    (void) func;
}
void WiFiClass::feedWatchdog() {
}


WiFiClass WiFi;
