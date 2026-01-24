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
#include "BLEClient.h"

//#include "BLEAdvertising.h"
#include <btstack.h>
#include <list>
#include <BluetoothLock.h>

#include "BLERemoteService.h"
#include "BLERemoteCharacteristic.h"

#include "BLECB.h"
#include "BLEDebug.h"

BLEClient::BLEClient() { }
BLEClient::~BLEClient() { }

uint32_t BLEClient::_timeout = 2000; // default TO

void BLEClient::setTimeout(uint32_t secs) {
    _timeout = secs * 1000;
}

void BLEClient::freeServices() {
    BluetoothLock l;
    for (auto s : _service) {
        delete s;
    }
    _service.clear();
}

bool BLEClient::connect(BLEAddress a, uint32_t timeout) {
    freeServices();

    do {
        BluetoothLock lock;
        if (!_gattInitted) {
            _gattInitted = true;
            gatt_client_init();
        }
        gap_connect(a.rawAddress(), (bd_addr_type_t)a.rawType());
    } while (0); // Run once

    timeout *= 1000; // to milliseconds
    uint32_t start = millis();

    // Wait for GATT connection
    while (millis() - start < timeout) {
        if (con_handle) {
            break;
        }
        delay(50);
    }
    if (!con_handle) {
        return false;
    }

    _waitingForCB = true;

    do {
        BluetoothLock lock;
        // Start service discovery, using this handler fcn (mediated thru BLE)
        gatt_client_discover_primary_services(GATTPACKETHANDLERCB(BLEClient, packetHandler), con_handle);
    } while (0);

    // Wait for svc scan done
    while (millis() - start < timeout) {
        if (!_waitingForCB) {
            break;
        }
        delay(50);
    }

    if (_waitingForCB) {
        //todo - stop scan
        freeServices();
        return false;
    }

    // Go through all services, make characteristics for them
    for (auto &x : _service) {
        _waitingForCB = true;
        x->scanCharacteristics(&_waitingForCB);
        while (millis() - start < timeout) {
            if (!_waitingForCB) {
                break;
            }
            delay(50);
        }
        if (_waitingForCB) {
            freeServices();
            return false;
        }
    }

    // Now get descriptors for all characteristics.  Whew, that's a lotta work
    for (auto &x : _service) {
        for (auto &y : x->_characteristic) {
            _waitingForCB = true;
            y->scanDescriptors(&_waitingForCB);
            while (millis() - start < timeout) {
                if (!_waitingForCB) {
                    break;
                }
                delay(50);
            }
            if (_waitingForCB) {
                freeServices();
                return false;
            }
        }
    }

    return true;
}

bool BLEClient::connect(BLEAdvertising &device, uint32_t timeoutsec) {
    return connect(device.getAddress());
}

void BLEClient::disconnect() {
    if (con_handle) {
        BluetoothLock lock;
        gap_disconnect(con_handle);
    } while (0);

    // con_handle will be cleared once we get the disconnection message, so wait for it
    while (con_handle) {
        delay(50);
    }
    _service.clear();
}


BLERemoteServiceList *BLEClient::services() {
    return &_service;
}

bool BLEClient::connected() {
    return con_handle ? true : false;
}


//    String toString();

bool BLEClient::begin() {
    return true;
}

void BLEClient::setCallbacks(BLEClientCallbacks *cb) {
    _clientCB = cb;
}

BLERemoteService *BLEClient::service(BLEUUID uuid) {
    for (auto &x : _service) {
        if (x->_uuid == uuid) {
            return x;
        }
    }
    return nullptr;
}

// Called by BLE class, from the BTStack hci packet callback
void BLEClient::packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size) {
    gatt_client_service_t service;
    gatt_client_characteristic_t characteristic;

    if (type != HCI_EVENT_PACKET) {
        return;
    }
    switch (hci_event_packet_get_type(packet)) {
    case HCI_EVENT_META_GAP:
        DEBUGBLE("HCI_EVENT_META_GAP\n");
        if (hci_event_gap_meta_get_subevent_code(packet) != GAP_SUBEVENT_LE_CONNECTION_COMPLETE) {
            break;
        }
        con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
        break;
    case HCI_EVENT_DISCONNECTION_COMPLETE:
        DEBUGBLE("HCI_EVENT_DISCONNECTION_COMPLETE\n");
        for (auto &s : _service) {
            s->disconnect();
            delete s;
        }
        con_handle = 0;
        if (_clientCB) {
            _clientCB->onDisconnect(this);
        }
        break;
    case GATT_EVENT_SERVICE_QUERY_RESULT:
        DEBUGBLE("GATT_EVENT_SERVICE_QUERY_RESULT\n");
        gatt_event_service_query_result_get_service(packet, &service);
        if (service.uuid16) {
            _service.push_back(new BLERemoteService(con_handle, BLEUUID(service.uuid16), (void *)&service));
        } else {
            _service.push_back(new BLERemoteService(con_handle, BLEUUID(service.uuid128), &service));
        }
        break;
    case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
        DEBUGBLE("GATT_EVENT_CHARACTERISTIC_QUERY_RESULT\n");
        gatt_event_characteristic_query_result_get_characteristic(packet, &characteristic);
        break;
    case GATT_EVENT_QUERY_COMPLETE:
        DEBUGBLE("GATT_EVENT_QUERY_COMPLETE\n");
        _waitingForCB = false;
        break;
    }
}
