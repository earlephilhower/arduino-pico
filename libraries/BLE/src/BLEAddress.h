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


#pragma once

#include <Arduino.h>

enum BLEAddressType {
    BLEPublicAddress = 0 /*BD_ADDR_TYPE_LE_PUBLIC*/,
    BLERandomAddress = 1 /*BD_ADDR_TYPE_LE_RANDOM*/, // TODO, not yet handled
};

typedef uint8_t BLEMAC[6];

class BLEAddress {
public:
    BLEAddress();
    BLEAddress(uint8_t *b, BLEAddressType type = BLEPublicAddress);
    BLEAddress(String a, BLEAddressType type = BLEPublicAddress);

    operator bool();
    bool operator==(const BLEAddress &o);
    uint8_t *rawAddress();

    String toString();

protected:
    BLEMAC _addr;
    BLEAddressType _type;
};
