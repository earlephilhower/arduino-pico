/*
    BLEAddress - Encapsulate a BT MAC address
    Copyright (c) 2026 Earle F. Philhower, III.  All rights reserved.

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


#include <Arduino.h>
#include "BLEAddress.h"


BLEAddress::BLEAddress() {
    bzero(_addr, sizeof(_addr));
    _type = BLEPublicAddress;
}

BLEAddress::BLEAddress(uint8_t *b, BLEAddressType type) {
    memcpy(_addr, b, sizeof(_addr));
    _type = type;
}

BLEAddress::BLEAddress(String a, BLEAddressType type) {
    const char *c = a.c_str();
    int idx = 0;
    while (*c && *(c + 1)) {
        char bf[3] = { c[0], c[1], 0 };
        unsigned int v;
        if (1 == sscanf(bf, "%ux", &v)) {
            _addr[idx++] = v & 0xff;
            if (idx == 6) {
                break;
            }
            c += 2;
        } else {
            c++;
        }
    }
    _type = type;
}

String BLEAddress::toString() {
    char buff[32];
    sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X", _addr[0], _addr[1], _addr[2], _addr[3], _addr[4], _addr[5]);
    return String(buff);
}

BLEAddress::operator bool() {
    for (int i = 0; i < 6; i++) {
        if (_addr[i]) {
            return true;
        }
    }
    return false;
}

bool BLEAddress::operator==(const BLEAddress &a) {
    return !memcmp(a._addr, _addr, sizeof(_addr));
}

uint8_t *BLEAddress::rawAddress() {
    return (uint8_t *)_addr;
}

int BLEAddress::rawType() {
    return (int)_type;
}
