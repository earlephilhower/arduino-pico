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

#include <Arduino.h>
#include <btstack.h>
#include <ble/att_db_util.h>
#include "BLEService.h"

BLEService::BLEService() {
}

BLEService::BLEService(BLEUUID prim) {
    _svc = prim;
}

BLEService::~BLEService() {
}

void BLEService::addCharacteristic(BLECharacteristic *c) {
    _characteristic.push_back(c);
}

void BLEService::addATTDB() {
    if (_svc.is16) {
        _svcHandle = att_db_util_add_service_uuid16(_svc.uuid16);
    } else {
        _svcHandle = att_db_util_add_service_uuid128(_svc.uuid128);
    }
    for (auto a : _characteristic) {
        a->addATTDB();
    }
}

// Called when the BLE connects to pass down the con_handle
void BLEService::setConHandle(uint16_t h) {
    con_handle = h;
    for (auto c : _characteristic) {
        c->setConHandle(h);
    }
}

// Called when the BLE report disconnect
void BLEService::disconnected() {
    for (auto c : _characteristic) {
        c->disconnected();
    }
}

int BLEService::handleRead(uint16_t attribute_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    int ret;
    for (auto c : _characteristic) {
        ret = c->handleRead(attribute_handle, offset, buffer, buffer_size);
        if (ret) {
            return ret;
        }
    }
    return 0;
}

int BLEService::handleWrite(uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    int ret;
    for (auto c : _characteristic) {
        ret = c->handleWrite(attribute_handle, transaction_mode, offset, buffer, buffer_size);
        if (ret) {
            return ret;
        }
    }
    return 0;
}
