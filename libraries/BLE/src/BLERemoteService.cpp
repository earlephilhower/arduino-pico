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

#include <Arduino.h>
#include "BLERemoteService.h"
#include <btstack.h>
#include <BluetoothLock.h>
#include "BLECB.h"


BLERemoteService::BLERemoteService(uint16_t con, BLEUUID uuid, const void /*gatt_client_service_t*/ *svc) {
    con_handle = con;
    _uuid = uuid;
    _gcs = malloc(sizeof(gatt_client_service_t));
    memcpy(_gcs, svc, sizeof(gatt_client_service_t));
}

BLERemoteService::~BLERemoteService() {
    BluetoothLock l;
    free(_gcs);
    for (auto c : _characteristic) {
        delete c;
    }
}

BLERemoteCharacteristic *BLERemoteService::characteristic(BLEUUID uuid) {
    for (auto &x : _characteristic) {
        if (x->_uuid == uuid) {
            return x;
        }
    }
    return nullptr;
}

void BLERemoteService::scanCharacteristics(volatile bool *flag) {
    if (!con_handle) {
        return;
    }
    _cbflag = flag;
    do {
        BluetoothLock lock;
        gatt_client_discover_characteristics_for_service(GATTPACKETHANDLERCB(BLERemoteService, packetHandler), con_handle, (gatt_client_service_t *)_gcs);
    } while (0);
}

void BLERemoteService::packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size) {
    gatt_client_characteristic_t characteristic;

    if (type != HCI_EVENT_PACKET) {
        return;
    }
    switch (hci_event_packet_get_type(packet)) {
    case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
        gatt_event_characteristic_query_result_get_characteristic(packet, &characteristic);
        _characteristic.push_back(new BLERemoteCharacteristic(con_handle, &characteristic));
        break;
    case GATT_EVENT_QUERY_COMPLETE:
        *_cbflag = false;
        break;
    }
}

void BLERemoteService::disconnect() {
    for (auto c : _characteristic) {
        c->disconnect();
    }
    con_handle = 0;
}

String BLERemoteService::toString() {
    String o = _uuid.toString();
    o += ";";
    return o;
}
