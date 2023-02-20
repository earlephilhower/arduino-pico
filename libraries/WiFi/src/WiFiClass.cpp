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
#include <pico/cyw43_arch.h>
#include <cyw43.h>
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include <map>
#include "WiFi.h"

// This is the real WiFi network object, we just tickle it to do our magic
#include <LwipEthernet.h>
static CYW43lwIP _wifi(1);

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


/*  Start WiFi connection for OPEN networks

    param ssid: Pointer to the SSID string.
*/
int WiFiClass::begin(const char* ssid) {
    return begin(ssid, nullptr);
}

int WiFiClass::beginBSSID(const char* ssid, const uint8_t *bssid) {
    return begin(ssid, nullptr, bssid);
}

#ifdef ARDUINO_RASPBERRY_PI_PICO_W
/*  Start WiFi connection with passphrase
    the most secure supported mode will be automatically selected

    param ssid: Pointer to the SSID string.
    param passphrase: Passphrase. Valid characters in a passphrase
          must be between ASCII 32-126 (decimal).
*/
int WiFiClass::begin(const char* ssid, const char *passphrase, const uint8_t *bssid) {
    // Simple ESP8266 compatibility hack
    if (_modeESP == WIFI_AP) {
        return beginAP(ssid, passphrase);
    }

    end();

    _ssid = ssid;
    _password = passphrase;
    if (bssid) {
        memcpy(_bssid, bssid, sizeof(_bssid));
    } else {
        bzero(_bssid, sizeof(_bssid));
    }
    _wifi.setSSID(_ssid.c_str());
    _wifi.setBSSID(_bssid);
    _wifi.setPassword(passphrase);
    _wifi.setTimeout(_timeout);
    _wifi.setSTA();
    _apMode = false;
    _wifiHWInitted = true;
    uint32_t start = millis(); // The timeout starts from network init, not network link up
    if (!_wifi.begin()) {
        return WL_IDLE_STATUS;
    }
    noLowPowerMode();
    // Enable CYW43 event debugging (make sure Debug Port is set)
    //cyw43_state.trace_flags = 0xffff;
    while (!_calledESP && ((millis() - start < (uint32_t)2 * _timeout)) && !connected()) {
        delay(10);
    }
    return status();
}

uint8_t WiFiClass::beginAP(const char *ssid) {
    return beginAP(ssid, nullptr);
}

uint8_t WiFiClass::beginAP(const char *ssid, uint8_t channel) {
    (void) channel;
    return beginAP(ssid, nullptr);
}

uint8_t WiFiClass::beginAP(const char *ssid, const char* passphrase, uint8_t channel) {
    (void) channel;
    return beginAP(ssid, passphrase);
}

uint8_t WiFiClass::beginAP(const char *ssid, const char* passphrase) {
    end();

    _ssid = ssid;
    _password = passphrase;
    _wifi.setSSID(_ssid.c_str());
    _wifi.setPassword(passphrase);
    _wifi.setTimeout(_timeout);
    _wifi.setAP();
    _apMode = true;
    if (!_wifi.begin()) {
        return WL_IDLE_STATUS;
    }
    noLowPowerMode();
    IPAddress gw = _wifi.gatewayIP();
    if (!gw.isSet()) {
        gw = IPAddress(192, 168, 42, 1);
    }
    IPAddress mask = _wifi.subnetMask();
    if (!mask.isSet()) {
        mask = IPAddress(255, 255, 255, 0);
    }
    config(gw);
    _dhcpServer = (dhcp_server_t *)malloc(sizeof(dhcp_server_t));
    if (!_dhcpServer) {
        // OOM
        return WL_IDLE_STATUS;
    }
    dhcp_server_init(_dhcpServer, gw, mask);

    _wifiHWInitted = true;

    return WL_CONNECTED;
}
#endif

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


bool WiFiClass::connected() {
    return (_apMode && _wifiHWInitted) || (_wifi.connected() && localIP().isSet() && (cyw43_wifi_link_status(&cyw43_state, _apMode ? 1 : 0) == CYW43_LINK_JOIN));
}

/*  Change Ip configuration settings disabling the dhcp client

    param local_ip: 	Static ip configuration
*/
void WiFiClass::config(IPAddress local_ip) {
    ip4_addr_set_u32(ip_2_ip4(&_wifi.getNetIf()->ip_addr), local_ip.v4());
}

/*  Change Ip configuration settings disabling the dhcp client

    param local_ip: 	Static ip configuration
    param dns_server:     IP configuration for DNS server 1
*/
void WiFiClass::config(IPAddress local_ip, IPAddress dns_server) {
    ip4_addr_set_u32(ip_2_ip4(&_wifi.getNetIf()->ip_addr), local_ip.v4());
    dns_setserver(0, dns_server);
}

/*  Change Ip configuration settings disabling the dhcp client

    param local_ip: 	Static ip configuration
    param dns_server:     IP configuration for DNS server 1
    param gateway : 	Static gateway configuration
*/
void WiFiClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway) {
    ip4_addr_set_u32(ip_2_ip4(&_wifi.getNetIf()->ip_addr), local_ip.v4());
    dns_setserver(0, dns_server);
    ip4_addr_set_u32(ip_2_ip4(&_wifi.getNetIf()->gw), gateway.v4());
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
        cyw43_wifi_leave(&cyw43_state, _apMode ? 1 : 0);
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
    if (!_wifiHWInitted) {
        _apMode = false;
        cyw43_wifi_set_up(&cyw43_state, _apMode ? 1 : 0, true, CYW43_COUNTRY_WORLDWIDE);
    }
    cyw43_wifi_get_mac(&cyw43_state, _apMode ? 1 : 0, mac);
    return mac;
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
#ifndef CYW43_IOCTL_GET_BSSID
#define CYW43_IOCTL_GET_BSSID ( (uint32_t)23 * 2 )
#endif

    if (_wifi.connected()) {
        cyw43_ioctl(&cyw43_state, CYW43_IOCTL_GET_BSSID, WL_MAC_ADDR_LENGTH, bssid, CYW43_ITF_STA);
    } else {
        memset(bssid, 0, WL_MAC_ADDR_LENGTH);
    }
    return bssid;
}

int WiFiClass::channel() {
#ifndef CYW43_IOCTL_GET_CHANNEL
#define CYW43_IOCTL_GET_CHANNEL 0x3a
#endif

    int32_t channel;
    if (_wifi.connected()) {
        cyw43_ioctl(&cyw43_state, CYW43_IOCTL_GET_CHANNEL, sizeof channel, (uint8_t *)&channel, CYW43_ITF_STA);
    } else {
        channel = -1;
    }
    return channel;
}


/*
    Return the current RSSI /Received Signal Strength in dBm)
    associated with the network

    return: signed value
*/
int32_t WiFiClass::RSSI() {
#ifndef CYW43_IOCTL_GET_RSSI
#define CYW43_IOCTL_GET_RSSI 0xFE
#endif

    int32_t rssi;
    if (_wifi.connected()) {
        cyw43_ioctl(&cyw43_state, CYW43_IOCTL_GET_RSSI, sizeof rssi, (uint8_t *)&rssi, CYW43_ITF_STA);
    } else {
        rssi = -255;
    }
    return rssi;
}

/*
    Return the Encryption Type associated with the network

    return: one value of wl_enc_type enum
*/
uint8_t WiFiClass::encryptionType() {
    // TODO - Driver does not return this?!
    if (_password == nullptr) {
        return ENC_TYPE_NONE;
    }
    return ENC_TYPE_AUTO;
}

//TODO - this can be in the class
static uint64_t _to64(uint8_t b[8]) {
    uint64_t x = 0;
    for (int i = 0; i < 6; i++) {
        x <<= 8LL;
        x |= b[i] & 255;
    }
    return x;
}

int WiFiClass::_scanCB(void *env, const cyw43_ev_scan_result_t *result) {
    WiFiClass *w = (WiFiClass *)env;
    if (result) {
        cyw43_ev_scan_result_t s;
        memcpy(&s, result, sizeof(s));
        w->_scan.insert_or_assign(_to64(s.bssid), s);
    }
    return 0;
}

/*
    Start scan WiFi networks available

    return: Number of discovered networks
*/
int8_t WiFiClass::scanNetworks(bool async) {
    cyw43_wifi_scan_options_t scan_options;
    memset(&scan_options, 0, sizeof(scan_options));
    _scan.clear();
    if (!_wifiHWInitted) {
        _apMode = false;
        cyw43_arch_enable_sta_mode();
        _wifiHWInitted = true;
    }
    int err = cyw43_wifi_scan(&cyw43_state, &scan_options, this, _scanCB);
    if (err) {
        return 0;
    }
    if (!async) {
        uint32_t now = millis();
        while (cyw43_wifi_scan_active(&cyw43_state) && (millis() - now < 10000)) {
            delay(10);
        }
        return _scan.size();
    } else {
        return -1;
    }
}

int8_t WiFiClass::scanComplete() {
    if (cyw43_wifi_scan_active(&cyw43_state)) {
        return -1;
    } else {
        return _scan.size();
    }
}

void WiFiClass::scanDelete() {
    _scan.clear();
}

/*
    Return the SSID discovered during the network scan.

    param networkItem: specify from which network item want to get the information

    return: ssid string of the specified item on the networks scanned list
*/
const char*WiFiClass::SSID(uint8_t networkItem) {
    if (networkItem >= _scan.size()) {
        return nullptr;
    }
    auto it = _scan.begin();
    for (int i = 0; i < networkItem; i++) {
        ++it;
    }
    return (const char *)it->second.ssid;
}

/*
    Return the encryption type of the networks discovered during the scanNetworks

    param networkItem: specify from which network item want to get the information

    return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
*/
uint8_t WiFiClass::encryptionType(uint8_t networkItem) {
    if (networkItem >= _scan.size()) {
        return ENC_TYPE_UNKNOWN;
    }
    auto it = _scan.begin();
    for (int i = 0; i < networkItem; i++) {
        ++it;
    }
    // TODO - the driver returns a small integer but does not actually provide a way of mapping that to the proper enc type.  My best guesses here...
    switch (it->second.auth_mode) {
    case 0: return ENC_TYPE_NONE;
    case 3: return ENC_TYPE_TKIP;
    case 5: return ENC_TYPE_CCMP;
    case 7: return ENC_TYPE_AUTO;
    }
    return ENC_TYPE_UNKNOWN;
}

uint8_t* WiFiClass::BSSID(uint8_t networkItem, uint8_t* bssid) {
    if (networkItem >= _scan.size()) {
        return nullptr;
    }
    auto it = _scan.begin();
    for (int i = 0; i < networkItem; i++) {
        ++it;
    }
    memcpy(bssid, it->second.bssid, 6);
    return bssid;
}

uint8_t WiFiClass::channel(uint8_t networkItem) {
    if (networkItem >= _scan.size()) {
        return 255;
    }
    auto it = _scan.begin();
    for (int i = 0; i < networkItem; i++) {
        ++it;
    }
    return it->second.channel;
}

/*
    Return the RSSI of the networks discovered during the scanNetworks

    param networkItem: specify from which network item want to get the information

    return: signed value of RSSI of the specified item on the networks scanned list
*/
int32_t WiFiClass::RSSI(uint8_t networkItem) {
    if (networkItem >= _scan.size()) {
        return -9999;
    }
    auto it = _scan.begin();
    for (int i = 0; i < networkItem; i++) {
        ++it;
    }
    return it->second.rssi;
}

/*
    Return Connection status.

    return: one of the value defined in wl_status_t
*/
uint8_t WiFiClass::status() {
    if (_apMode && _wifiHWInitted) {
        return WL_CONNECTED;
    }
    switch (cyw43_wifi_link_status(&cyw43_state, _apMode ? 1 : 0)) {
    case CYW43_LINK_DOWN: return WL_IDLE_STATUS;
    case CYW43_LINK_JOIN: return localIP().isSet() ? WL_CONNECTED : WL_DISCONNECTED;
    case CYW43_LINK_FAIL: return WL_CONNECT_FAILED;
    case CYW43_LINK_NONET: return WL_CONNECT_FAILED;
    case CYW43_LINK_BADAUTH: return WL_CONNECT_FAILED;
    }
    return WL_NO_MODULE;
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

int WiFiClass::hostByName(const char* aHostname, IPAddress& aResult, int timeout_ms) {
    return _wifi.hostByName(aHostname, aResult, timeout_ms);
}

// TODO
unsigned long WiFiClass::getTime() {
    return millis();
}

void WiFiClass::aggressiveLowPowerMode() {
    cyw43_wifi_pm(&cyw43_state, CYW43_AGGRESSIVE_PM);
}

void WiFiClass::defaultLowPowerMode() {
    cyw43_wifi_pm(&cyw43_state, CYW43_DEFAULT_PM);
}

// The difference between the default CYW43_DEFAULT_PM (0xA11142) and not low power (0xA11140) is that it changed from "Powersave mode on specified interface with High throughput" to "No Powersave mode". All other parameters stayed the same.
void WiFiClass::noLowPowerMode() {
    cyw43_wifi_pm(&cyw43_state, 0xA11140);
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
