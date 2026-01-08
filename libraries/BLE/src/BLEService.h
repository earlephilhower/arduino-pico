/*
    BLEService - Implements a BLE service, holds characteristics
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
#include <list>
#include "BLEUUID.h"
#include "BLECharacteristic.h"

class BLEService {
public:
    BLEService();
    BLEService(BLEUUID prim);
    virtual ~BLEService();

    void addCharacteristic(BLECharacteristic *c);
    void addATTDB();

protected:
    friend class BLEServer;
    // Called when the BLE connects to pass down the con_handle
    virtual void setConHandle(uint16_t h);

    // Called when the BLE report disconnect
    virtual void disconnected();

    int handleRead(uint16_t attribute_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
    int handleWrite(uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

    uint16_t _svcHandle;
    uint16_t con_handle = 0;
    std::list<BLECharacteristic *> _characteristic;
    BLEUUID _svc;  // Only primary service supported
};
