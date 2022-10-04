/*
    WiFiNTP.h - Simple NTP wrapper for LWIP
    Copyright (c) 2022 Earle F. Philhower, III.  All rights reserved.

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
#include <time.h>
#include <lwip/apps/sntp.h>

class NTPClass {
public:
    NTPClass() { }

    ~NTPClass() {
        sntp_stop();
    }

    void begin(IPAddress server, int timeout = 3600) {
        (void) timeout;
        sntp_stop();
        if (server.isSet()) {
            sntp_setserver(0, server);
            sntp_setoperatingmode(SNTP_OPMODE_POLL);
            sntp_init();
            _running = true;
        }
    }

    void begin(IPAddress s1, IPAddress s2, int timeout = 3600) {
        (void) timeout;
        sntp_stop();
        if (s1.isSet()) {
            sntp_setserver(0, s1);
        }
        if (s2.isSet()) {
            sntp_setserver(1, s2);
        }
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_init();
        _running = true;
    }

    void begin(const char *server, int timeout = 3600) {
        IPAddress addr;
        if (WiFi.hostByName(server, addr)) {
            begin(addr, timeout);
        }
        _running = true;
    }

    void begin(const char *s1, const char *s2, int timeout = 3600) {
        IPAddress a1, a2;
        if (WiFi.hostByName(s1, a1)) {
            if (WiFi.hostByName(s2, a2)) {
                begin(a1, a2, timeout);
            } else {
                begin(a1, timeout);
            }
        }
        _running = true;
    }

    bool waitSet(uint32_t timeout = 10000) {
        return waitSet(nullptr, timeout);
    }

    bool waitSet(void (*cb)(), uint32_t timeout = 10000) {
        if (!running()) {
            begin("pool.ntp.org");
        }
        uint32_t start = millis();
        while ((time(nullptr) < 10000000) && (millis() - start < timeout)) {
            delay(100);
            if (cb) {
                cb();
            }
        }
        return time(nullptr) < 10000000;
    }

    bool running() {
        return _running;
    }

private:
    bool _running = false;
};

// ESP8266 compat
#define configTime(timeout, tzoffsec, server1, server2) NTP.begin(server1, server2, timeout)

extern NTPClass NTP;
