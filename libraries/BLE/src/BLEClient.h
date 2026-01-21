/*
    BLEClient- BLE top-level client (connects to other ddevices)
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

#include "BLEAdvertising.h"
#include "BLERemoteService.h"
#include "BLERemoteCharacteristic.h"


class BLEClient;

class BLEClientCallbacks {
public:
    virtual void onDisconnect(BLEClient *p) {
        printf("BLEServer disconnect: %p\n", p);
    }
};

class BLEClient {
public:
    BLEClient();
    ~BLEClient();

    bool connect(BLEAddress a, uint32_t timeout = 10);
    bool connect(BLEAdvertising &device, uint32_t timeoutsec = 10);
    void disconnect();

    BLERemoteServiceList *services();
    BLERemoteService *service(BLEUUID uuid);

    bool connected();

    bool begin();

    void setCallbacks(BLEClientCallbacks *cb);

    void setTimeout(uint32_t secs);

protected:
    friend class BLEClass;
    friend class BLERemoteService;
    friend class BLERemoteCharacteristic;

    // Called by BLE class, from the BTStack hci packet callback
    void packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size);

    // delete all the elements of the service list
    void freeServices();

    friend class BLEClass;
    bool _gattInitted = false;
    BLERemoteServiceList _service;
    volatile bool _waitingForCB = false;
    BLEClientCallbacks *_clientCB = nullptr;
    volatile uint16_t con_handle = 0;
    static uint32_t _timeout;
};
