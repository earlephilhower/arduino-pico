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

uint8_t WiFiMulti::run(uint32_t to) {

    // If we're already connected, don't re-scan/etc.
    if (WiFi.status() == WL_CONNECTED) {
        return WL_CONNECTED;
    }

    int cnt = WiFi.scanNetworks();
    if (!cnt) {
        return WL_DISCONNECTED;
    }

    // Find the highest RSSI network in our list.  Probably more efficient searches, but the list
    // of APs will have < 5 in > 99% of the cases so it's a don't care.
    int maxRSSID = -999;
    std::list<struct _AP>::iterator hit;
    bool found = false;
    for (int i = 0; i < cnt; i++) {
        if (WiFi.RSSI(i) > maxRSSID) {
            for (auto j = _list.begin(); j != _list.end(); j++) {
                DEBUGV("[WIFIMULTI] Checking for '%s' at %d\n", WiFi.SSID(i), WiFi.RSSI(i));
                if (!strcmp(j->ssid, WiFi.SSID(i))) {
                    hit = j;
                    maxRSSID = WiFi.RSSI(i);
                    found = true;
                }
            }
        }
    }
    if (!found) {
        return WL_DISCONNECTED;
    }

    // Connect!
    DEBUGV("[WIFIMULTI] Connecting to '%s' and '%s'\n", hit->ssid, hit->pass);
    uint32_t start = millis();
    if (hit->pass) {
        WiFi.begin(hit->ssid, hit->pass);
    } else {
        WiFi.begin(hit->ssid);
    }
    while (!WiFi.connected() && (millis() - start < to)) {
        delay(5);
    }
    return WiFi.status();
}
