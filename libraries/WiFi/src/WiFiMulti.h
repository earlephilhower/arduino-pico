/*
    WiFiMulti.h - Choose best RSSI and connect
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

#pragma once

#include <list>
#include <stdint.h>
#include "wl_definitions.h"

class WiFiMulti {
public:
    WiFiMulti();
    ~WiFiMulti();

    bool addAP(const char *ssid, const char *pass = nullptr);

    uint8_t run(uint32_t to = 10000);

private:
    struct _AP {
        char *ssid;
        char *pass;
    };
    std::list<struct _AP> _list;
};
