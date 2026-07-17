/*
    BLEBeacon - Implements a simple BLE beacon server
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
#include "BLE.h"
#include "BLEBeacon.h"

BLEBeacon::BLEBeacon() : BLEService() {
}

void BLEBeacon::setUUID(BLEUUID u) {
    _uuid = u;
}

void BLEBeacon::setCompanyID(uint16_t id) {
    _companyID = id;
}

void BLEBeacon::setMajorMinor(uint16_t major, uint16_t minor) {
    _major = major;
    _minor = minor;
}

void BLEBeacon::setTXPower(int8_t p) {
    _txpwr = p;
}

bool BLEBeacon::begin() {
    if (!_uuid || _uuid.is16) {
        return false;
    }
    BLE.advertising()->setName("");  // No name, no enough space in the ADV data
    uint8_t ibeaconmanuf[25] = {
        (uint8_t)(_companyID & 0xff), (uint8_t)(_companyID >> 8),
        0x02, 0x15,
        _uuid.uuid128[0], _uuid.uuid128[1], _uuid.uuid128[2], _uuid.uuid128[3], _uuid.uuid128[4], _uuid.uuid128[5], _uuid.uuid128[6], _uuid.uuid128[7],
        _uuid.uuid128[8], _uuid.uuid128[9], _uuid.uuid128[10], _uuid.uuid128[11], _uuid.uuid128[12], _uuid.uuid128[13], _uuid.uuid128[14], _uuid.uuid128[15],
        (uint8_t)(_major >> 8), (uint8_t)(_major & 0xff),
        (uint8_t)(_minor >> 8), (uint8_t)(_minor & 0xff),
        (uint8_t)_txpwr
    };
    BLE.advertising()->setManufacturerData(ibeaconmanuf, sizeof(ibeaconmanuf));
    BLE.startAdvertising();
    return true;
}
