/*
    Bluetooth device helper class

    Copyright (c) 2024 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

class BTDeviceInfo {
public:
    BTDeviceInfo(uint32_t dc, const uint8_t addr[6], int rssi, const char *name) {
        _deviceClass = dc;
        memcpy(_address, addr, sizeof(_address));
        sprintf(_addressString, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
        _rssi = rssi;
        _name = strdup(name);
    }
    // Copy constructor to ensure we deep-copy the string
    BTDeviceInfo(const BTDeviceInfo &b) {
        _deviceClass = b._deviceClass;
        memcpy(_address, b._address, sizeof(_address));
        memcpy(_addressString, b._addressString, sizeof(_addressString));
        _rssi = b._rssi;
        _name = strdup(b._name);
    }
    ~BTDeviceInfo() {
        free(_name);
    }
    uint32_t deviceClass() {
        return _deviceClass;
    }
    const uint8_t *address() {
        return _address;
    }
    const char *addressString() {
        return _addressString;
    }
    int rssi() {
        return _rssi;
    }
    const char *name() {
        return _name;
    }
private:
    uint32_t _deviceClass;
    uint8_t _address[6];
    char _addressString[18];
    int8_t _rssi;
    char *_name;
};
