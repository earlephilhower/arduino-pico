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

#include "BLERemoteCharacteristic.h"

class BLERemoteService {
public:
    ~BLERemoteService();

    // We canna copy this object, cap'n.  Handles/etc. for btstack are singletons
    BLERemoteService(const BLERemoteService& x) = delete;
    BLERemoteService operator=(const BLERemoteService& x) = delete;

    BLERemoteCharacteristic *characteristic(BLEUUID uuid);

protected:
    friend class BLEClient;

    // Only BLEClient can make one of these, users never need to do so
    BLERemoteService(uint16_t con, BLEUUID uuid, const void /*gatt_client_service_t*/ *svc);

    void scanCharacteristics(volatile bool *flag);

    void packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size);

    void disconnect();
    String toString();

    BLERemoteCharacteristicList _characteristic;
    volatile bool *_cbflag = nullptr;
    BLEUUID _uuid;
    volatile uint16_t con_handle;

    // Use blobs to avid including btstack.h because it breaks TinyUSB compilation in many cases
    // gatt_client_service_t _gattService = {};
    void *_gcs = nullptr;
};

typedef std::list<BLERemoteService *> BLERemoteServiceList;
