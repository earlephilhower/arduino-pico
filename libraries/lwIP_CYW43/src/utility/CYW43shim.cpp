/*
    WiFi <-> LWIP driver for the CYG43 chip on the Raspberry Pico W

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

#include "CYW43shim.h"
extern "C" {
#include <cyw43.h>
#include <cyw43_stats.h>
}
#include <pico/cyw43_arch.h>
#include <Arduino.h>

// From cyw43_ctrl.c
#define WIFI_JOIN_STATE_KIND_MASK (0x000f)
#define WIFI_JOIN_STATE_ACTIVE  (0x0001)
#define WIFI_JOIN_STATE_FAIL    (0x0002)
#define WIFI_JOIN_STATE_NONET   (0x0003)
#define WIFI_JOIN_STATE_BADAUTH (0x0004)
#define WIFI_JOIN_STATE_AUTH    (0x0200)
#define WIFI_JOIN_STATE_LINK    (0x0400)
#define WIFI_JOIN_STATE_KEYED   (0x0800)
#define WIFI_JOIN_STATE_ALL     (0x0e01)

netif *CYW43::_netif = nullptr;

struct netif *__getCYW43Netif() {
    return CYW43::_netif;
}

CYW43::CYW43(int8_t cs, arduino::SPIClass& spi, int8_t intrpin) {
    (void) cs;
    (void) spi;
    (void) intrpin;
    _netif = nullptr;
    bzero(_bssid, sizeof(_bssid));
}

bool CYW43::begin(const uint8_t* address, netif* netif) {
    (void) address;
    _netif = netif;
    _self = &cyw43_state;

    if (!_ap) {
        _itf = 0;
        cyw43_arch_enable_sta_mode();
        cyw43_wifi_get_mac(_self, _itf, netif->hwaddr);

        auto authmode = CYW43_AUTH_WPA2_AES_PSK;
        if (_password == nullptr) {
            authmode = CYW43_AUTH_OPEN;
        }

        // Not currently possible to hook up igmp_mac_filter and mld_mac_filter
        // TODO: implement igmp_mac_filter and mld_mac_filter
        // Implement cyw43_set_allmulti(_self, true) using exposed ioctl call (may not be functional in SDK 1.5?)
        uint8_t allmulti_true[] = { 'a', 'l', 'l', 'm', 'u', 'l', 't', 'i', 0, 1, 0, 0, 0 };
        cyw43_ioctl(&cyw43_state, CYW43_IOCTL_SET_VAR, sizeof allmulti_true, allmulti_true, CYW43_ITF_STA);
        // Add MDNS multicast MAC addresses manually, thanks Wikipedia!
        uint8_t mdnsV4[] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0xFB};
        cyw43_wifi_update_multicast_filter(&cyw43_state, mdnsV4, true);
#if LWIP_IPV6
        uint8_t mdnsV6[] = {0x33, 0x33, 0x00, 0x00, 0x00, 0xFB};
        cyw43_wifi_update_multicast_filter(&cyw43_state, mdnsV6, true);
#endif

        if (_bssid[0] | _bssid[1] | _bssid[2] | _bssid[3] | _bssid[4] | _bssid[5]) {
            if (cyw43_arch_wifi_connect_bssid_timeout_ms(_ssid, _bssid, _password, authmode, _timeout)) {
                return false;
            } else {
                return true;
            }
        } else {
            if (cyw43_arch_wifi_connect_timeout_ms(_ssid, _password, authmode, _timeout)) {
                return false;
            } else {
                return true;
            }
        }
    } else {
        _itf = 1;
        cyw43_arch_enable_ap_mode(_ssid, _password, _password ? CYW43_AUTH_WPA2_AES_PSK : CYW43_AUTH_OPEN);
        cyw43_wifi_get_mac(_self, _itf, netif->hwaddr);
        return true;
    }
}

void CYW43::end() {
    _netif = nullptr;
    cyw43_deinit(&cyw43_state);
}
int fails = 0;
int calls = 0;
uint16_t CYW43::sendFrame(const uint8_t* data, uint16_t datalen) {
    calls++;
    if (0 == cyw43_send_ethernet(_self, _itf, datalen, data, false)) {
        return datalen;
    }
    fails++;
    return 0;
}

uint16_t CYW43::readFrame(uint8_t* buffer, uint16_t bufsize) {
    // This is the polling method, but we hand this thru the interrupts
    (void) buffer;
    (void) bufsize;
    return 0;
}
