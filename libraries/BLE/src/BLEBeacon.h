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


#pragma once

#include <Arduino.h>
#include "BLEUUID.h"
#include "BLEService.h"

class BLEBeacon : public BLEService {
public:
    BLEBeacon();

    void setUUID(BLEUUID u);
    void setCompanyID(uint16_t id);
    void setMajorMinor(uint16_t major, uint16_t minor);
    void setTXPower(int8_t p);

    bool begin();

private:
    BLEUUID _uuid;
    uint16_t _companyID = 0x004c;
    uint16_t _major = 1;
    uint16_t _minor = 1;
    int8_t _txpwr = -50;  // Power in DB at 1m from device
};
