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
    // Classic Bluetooth Device
    BTDeviceInfo(uint32_t dc, const uint8_t addr[6], int rssi, const char *name) {
        _deviceClass = dc;
        memcpy(_address, addr, sizeof(_address));
        _addressType = -1;
        sprintf(_addressString, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
        _rssi = rssi;
        strncpy(_name, name, sizeof(_name));
        _name[sizeof(_name) - 1] = 0;
    }

    // Bluetooth BLE Device
    BTDeviceInfo(uint32_t dc, const uint8_t addr[6], int addressType, int rssi, const char *name, size_t nameLen) {
        _deviceClass = dc;
        memcpy(_address, addr, sizeof(_address));
        sprintf(_addressString, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
        _addressType = addressType;
        _rssi = rssi;
        memcpy(_name, name, std::min(nameLen, sizeof(_name)));
        _name[std::min(nameLen, sizeof(_name) - 1)] = 0;
    }

    ~BTDeviceInfo() {
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

    int addressType() {
        return _addressType;
    }

private:
    uint32_t _deviceClass;
    uint8_t _address[6];
    int _addressType;
    char _addressString[18];
    int8_t _rssi;
    char _name[241];
};
