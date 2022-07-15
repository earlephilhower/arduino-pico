/*
    LwipIntf.h

    Arduino interface for lwIP generic callbacks and functions

    Original Copyright (c) 2020 esp8266 Arduino All rights reserved.
    This file is part of the esp8266 Arduino core environment.

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

#include <lwip/netif.h>
#include <IPAddress.h>

#include <functional>

class LwipIntf {
public:
    using CBType = std::function<void(netif*)>;

    static bool stateUpCB(LwipIntf::CBType&& cb);

    // reorder WiFi.config() parameters for a esp8266/official Arduino dual-compatibility API
    // args     | esp order  arduino order
    // ----     + ---------  -------------
    // local_ip | local_ip   local_ip
    // arg1     | gateway    dns1
    // arg2     | netmask    [Agateway
    // arg3     | dns1       netmask
    //
    // result stored into gateway/netmask/dns1
    static bool ipAddressReorder(const arduino::IPAddress& local_ip, const arduino::IPAddress& arg1,
                                 const arduino::IPAddress& arg2, const arduino::IPAddress& arg3, arduino::IPAddress& gateway,
                                 arduino::IPAddress& netmask, arduino::IPAddress& dns1);

    arduino::String hostname();

    bool hostname(const arduino::String& aHostname) {
        return hostname(aHostname.c_str());
    }

    bool hostname(const char* aHostname);

    // ESP32 API compatibility
    bool setHostname(const char* aHostName) {
        return hostname(aHostName);
    }

    // ESP32 API compatibility
    const char* getHostname();

protected:
    static bool stateChangeSysCB(LwipIntf::CBType&& cb);
};
