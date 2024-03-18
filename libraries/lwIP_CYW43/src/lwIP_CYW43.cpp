/*
    WiFiClass methods implementations for the CYG43 chip

    Copyright (c) 2022 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include "lwIP_CYW43.h"
#include <pico/cyw43_arch.h>

CYW43lwIP::CYW43lwIP(int8_t cs) : LwipIntfDev<CYW43>(cs) {

}

void CYW43lwIP::end() {
    cyw43_wifi_leave(&cyw43_state, _ap ? 1 : 0);
    LwipIntfDev<CYW43>::end();
}

bool CYW43lwIP::connected() {
    return (LwipIntfDev<CYW43>::connected() && (cyw43_wifi_link_status(&cyw43_state, 0) == CYW43_LINK_JOIN));
}

uint8_t* CYW43lwIP::macAddress(bool apMode, uint8_t* mac) {
    cyw43_wifi_get_mac(&cyw43_state, apMode ? 1 : 0, mac);
    return mac;
}

uint8_t* CYW43lwIP::BSSID(uint8_t* bssid) {
#ifndef CYW43_IOCTL_GET_BSSID
#define CYW43_IOCTL_GET_BSSID ( (uint32_t)23 * 2 )
#endif

    if (connected()) {
        cyw43_ioctl(&cyw43_state, CYW43_IOCTL_GET_BSSID, WL_MAC_ADDR_LENGTH, bssid, CYW43_ITF_STA);
    } else {
        memset(bssid, 0, WL_MAC_ADDR_LENGTH);
    }
    return bssid;
}

int CYW43lwIP::channel() {
#ifndef CYW43_IOCTL_GET_CHANNEL
#define CYW43_IOCTL_GET_CHANNEL 0x3a
#endif

    int32_t channel;
    if (connected()) {
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
int32_t CYW43lwIP::RSSI() {
#ifndef CYW43_IOCTL_GET_RSSI
#define CYW43_IOCTL_GET_RSSI 0xFE
#endif

    int32_t rssi;
    if (connected()) {
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
uint8_t CYW43lwIP::encryptionType() {
    // TODO - Driver does not return this?!
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

int CYW43lwIP::_scanCB(void *env, const cyw43_ev_scan_result_t *result) {
    CYW43lwIP *w = (CYW43lwIP *)env;
    if (result) {
        cyw43_ev_scan_result_t s;
        memcpy(&s, result, sizeof(s));
        w->_scan.insert_or_assign(_to64(s.bssid), s);
    }
    return 0;
}

int8_t CYW43lwIP::scanNetworks(bool async) {
    cyw43_wifi_scan_options_t scan_options;
    memset(&scan_options, 0, sizeof(scan_options));
    _scan.clear();
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

int8_t CYW43lwIP::scanComplete() {
    if (cyw43_wifi_scan_active(&cyw43_state)) {
        return -1;
    } else {
        return _scan.size();
    }
}

void CYW43lwIP::scanDelete() {
    _scan.clear();
}

const char*CYW43lwIP::SSID(uint8_t networkItem) {
    if (networkItem >= _scan.size()) {
        return nullptr;
    }
    auto it = _scan.begin();
    for (int i = 0; i < networkItem; i++) {
        ++it;
    }
    return (const char *)it->second.ssid;
}

uint8_t CYW43lwIP::encryptionType(uint8_t networkItem) {
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

uint8_t* CYW43lwIP::BSSID(uint8_t networkItem, uint8_t* bssid) {
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

uint8_t CYW43lwIP::channel(uint8_t networkItem) {
    if (networkItem >= _scan.size()) {
        return 255;
    }
    auto it = _scan.begin();
    for (int i = 0; i < networkItem; i++) {
        ++it;
    }
    return it->second.channel;
}

int32_t CYW43lwIP::RSSI(uint8_t networkItem) {
    if (networkItem >= _scan.size()) {
        return -9999;
    }
    auto it = _scan.begin();
    for (int i = 0; i < networkItem; i++) {
        ++it;
    }
    return it->second.rssi;
}

uint8_t CYW43lwIP::status() {
    switch (cyw43_wifi_link_status(&cyw43_state, 0)) {
    case CYW43_LINK_DOWN: return WL_IDLE_STATUS;
    case CYW43_LINK_JOIN: return localIP().isSet() ? WL_CONNECTED : WL_DISCONNECTED;
    case CYW43_LINK_FAIL: return WL_CONNECT_FAILED;
    case CYW43_LINK_NONET: return WL_CONNECT_FAILED;
    case CYW43_LINK_BADAUTH: return WL_CONNECT_FAILED;
    }
    return WL_NO_MODULE;
}

void CYW43lwIP::noLowPowerMode() {
    cyw43_wifi_pm(&cyw43_state, 0xA11140);
}
