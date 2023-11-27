/*
    WiFiMulti.cpp - Choose best RSSI and connect
    Copyright (c) 2022 Earle F. Philhower, III

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

    Modified by Ivan Grokhotkov, January 2015 - esp8266 support
*/

#include "WiFi.h"
#include <string.h>
#include <algorithm>

WiFiMulti::WiFiMulti() {
}

WiFiMulti::~WiFiMulti() {
    while (!_list.empty()) {
        struct _AP ap = _list.front();
        _list.pop_front();
        free(ap.ssid);
        free(ap.pass);
    }
}

bool WiFiMulti::addAP(const char *ssid, const char *pass) {
    struct _AP ap;
    if (!ssid) {
        return false;
    }
    ap.ssid = strdup(ssid);
    if (!ap.ssid) {
        return false;
    }
    if (pass) {
        ap.pass = strdup(pass);
        if (!ap.pass) {
            free(ap.ssid);
            return false;
        }
    } else {
        ap.pass = nullptr;
    }
    DEBUGV("[WIFIMULTI] Adding: '%s' %s' to list\n", ap.ssid, ap.pass);
    _list.push_front(ap);
    return true;
}

void WiFiMulti::clearAPList() {
    while (!_list.empty()) {
        struct _AP ap = _list.front();
        _list.pop_front();
        free(ap.ssid);
        free(ap.pass);
    }
}

uint8_t WiFiMulti::run(uint32_t to) {
    struct _scanAP {
        char *ssid;
        char *psk;
        uint8_t bssid[6];
        int rssi;
    };
    std::list<struct _scanAP> _scanList;

    // If we're already connected, don't re-scan/etc.
    if (WiFi.status() == WL_CONNECTED) {
        return WL_CONNECTED;
    }

    DEBUGV("[WIFIMULTI] Rescanning to build new list of APs\n");
    int cnt = WiFi.scanNetworks();
    if (!cnt) {
        return WL_DISCONNECTED;
    }

    // Add all matching ones to the scanList
    for (int i = 0; i < cnt; i++) {
        for (auto j = _list.begin(); j != _list.end(); j++) {
            if (!strcmp(j->ssid, WiFi.SSID(i))) {
                _scanAP itm;
                itm.ssid = j->ssid;
                itm.psk = j->pass;
                WiFi.BSSID(i, itm.bssid);
                itm.rssi = WiFi.RSSI(i);
                _scanList.push_front(itm);
            }
        }
    }
    // Sort by RSSI using C++ lambda magic
    _scanList.sort([](const struct _scanAP & a, const struct _scanAP & b) {
        return a.rssi > b.rssi;
    });
    for (auto j = _scanList.begin(); j != _scanList.end(); j++) {
        DEBUGV("[WIFIMULTI] scanList: SSID: '%s' -- BSSID: '%02X%02X%02X%02X%02X%02X' -- RSSI: %d\n", j->ssid,
               j->bssid[0], j->bssid[1], j->bssid[2], j->bssid[3], j->bssid[4], j->bssid[5], j->rssi);
    }

    // Attempt to connect to each (will be in order of decreasing RSSI)
    for (auto j = _scanList.begin(); j != _scanList.end(); j++) {
        DEBUGV("[WIFIMULTI] Connecting to: SSID: '%s' -- BSSID: '%02X%02X%02X%02X%02X%02X' -- RSSI: %d\n", j->ssid,
               j->bssid[0], j->bssid[1], j->bssid[2], j->bssid[3], j->bssid[4], j->bssid[5], j->rssi);
        uint32_t start = millis();
        if (j->psk) {
            WiFi.begin(j->ssid, j->psk, j->bssid);
        } else {
            WiFi.beginBSSID(j->ssid, j->bssid);
        }
        while (!WiFi.connected() && (millis() - start < to)) {
            delay(5);
        }
        if (WiFi.status() == WL_CONNECTED) {
            return WL_CONNECTED;
        }
    }

    // Failed at this point...
    return WiFi.status();
}
