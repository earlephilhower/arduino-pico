/*
    BLEServer - BLE top-level server (holds services)
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
#include "BLEService.h"
#include "BLEAdvertising.h"

class BLEServer;

class BLEServerCallbacks {
public:
    virtual void onConnect(BLEServer *p) {
        printf("BLEServer connect: %p\n", p);
    }
    virtual void onDisconnect(BLEServer *p) {
        printf("BLEServer disconnect: %p\n", p);
    }
};

class BLEServer {
public:
    virtual ~BLEServer();

    // We canna copy this object, cap'n.  Handles/etc. for btstack are singletons
    BLEServer(const BLEServer& x) = delete;
    BLEServer operator=(const BLEServer& x) = delete;

    void setName(const char *name);

    void addService(BLEService *s);

    String toString();

    bool begin();

    bool setCallbacks(BLEServerCallbacks *cb);

protected:
    friend class BLEClass;

    // Only BLEClass can make me
    BLEServer();
    void prepareAdvertising(BLEAdvertising *adv);
    void setSecurity(BLESecurityMode m);

    uint16_t readHandler(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
    int writeHandler(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
    void packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size);

    std::list<BLEService *> _svc;
    BLEServerCallbacks *_serverCB = nullptr;
    char *_name = nullptr;
    uint16_t con_handle = 0;
    uint16_t _nameHandle = 0;
    BLESecurityMode _secMode = BLESecurityNone;
};
