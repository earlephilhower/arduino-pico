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
#include "BLERemoteCharacteristic.h"
#include <btstack.h>
#include <BluetoothLock.h>
#include "BLECB.h"
#include "BLECharacteristic.h"
#include "BLEClient.h" // timeout

BLERemoteCharacteristic::BLERemoteCharacteristic(uint16_t con, const void /*gatt_client_characteristic_t*/ *cha) {
    _gcc = malloc(sizeof(gatt_client_characteristic_t));
    _gccd = malloc(sizeof(gatt_client_characteristic_descriptor_t) * 4);
    _gcn = malloc(sizeof(gatt_client_notification_t));

    con_handle = con;
    gatt_client_characteristic_t *ch = (gatt_client_characteristic_t *)cha;
    _uuid = ch->uuid16 ? BLEUUID(ch->uuid16) : BLEUUID(ch->uuid128);
    DEBUGBLE("%s\n", _uuid.toString().c_str());
    memcpy(_gcc, cha, sizeof(gatt_client_characteristic_t));
}

BLERemoteCharacteristic::~BLERemoteCharacteristic() {
    free(_gcc);
    free(_gccd);
    free(_gcn);
    free(_remoteValue);
}

void BLERemoteCharacteristic::setCallbacks(BLERemoteCharacteristicCallbacks *cb) {
    _charCB = cb;
}


void BLERemoteCharacteristic::scanDescriptors(volatile bool *flag) {
    _cbflag = flag;
    if (con_handle) {
        BluetoothLock lock;
        gatt_client_discover_characteristic_descriptors(GATTPACKETHANDLERCB(BLERemoteCharacteristic, packetHandler), con_handle, (gatt_client_characteristic_t *)_gcc) ;
    }
}

void BLERemoteCharacteristic::packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size) {
    if (type != HCI_EVENT_PACKET) {
        return;
    }

    gatt_client_characteristic_descriptor_t *_gattDescriptor = (gatt_client_characteristic_descriptor_t *)_gccd;
    gatt_client_characteristic_t *_gattCharacteristic = (gatt_client_characteristic_t *)_gcc;

    switch (hci_event_packet_get_type(packet)) {
    case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT:
        DEBUGBLE("GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT\n");
        _remoteValueLen = gatt_event_characteristic_value_query_result_get_value_length(packet);
        _remoteValue = realloc(_remoteValue, _remoteValueLen);
        memcpy(_remoteValue, gatt_event_characteristic_value_query_result_get_value(packet), _remoteValueLen);
        break;

    case GATT_EVENT_ALL_CHARACTERISTIC_DESCRIPTORS_QUERY_RESULT:
        DEBUGBLE("GATT_EVENT_ALL_CHARACTERISTIC_DESCRIPTORS_QUERY_RESULT\n");
        gatt_event_all_characteristic_descriptors_query_result_get_characteristic_descriptor(packet, &_gattDescriptor[_gattDescriptors++]);
        break;

    case GATT_EVENT_CHARACTERISTIC_DESCRIPTOR_QUERY_RESULT:
        DEBUGBLE("GATT_EVENT_CHARACTERISTIC_DESCRIPTOR_QUERY_RESULT\n");
        _description = (char *)malloc(gatt_event_characteristic_descriptor_query_result_get_descriptor_length(packet) + 1);
        memcpy(_description, gatt_event_characteristic_descriptor_query_result_get_descriptor(packet), gatt_event_characteristic_descriptor_query_result_get_descriptor_length(packet));
        _description[gatt_event_characteristic_descriptor_query_result_get_descriptor_length(packet)] = 0;
        break;

    case GATT_EVENT_NOTIFICATION:
        DEBUGBLE("GATT_EVENT_NOTIFICATION\n");
        if (gatt_event_notification_get_value_handle(packet) == _gattCharacteristic->value_handle) {
            if (_charCB) {
                _charCB->onNotify(this, gatt_event_notification_get_value(packet), gatt_event_notification_get_value_length(packet));
            }
            if (_cbFn) {
                (_cbFn)(this, gatt_event_notification_get_value(packet), gatt_event_notification_get_value_length(packet));
            }
        } else {
            DEBUGBLE("Got unexpected notification CB!\n");
        }
        break;

    case GATT_EVENT_QUERY_COMPLETE:
        DEBUGBLE("GATT_EVENT_QUERY_COMPLETE\n");
        *_cbflag = false;
        break;
    }
}

void BLERemoteCharacteristic::onNotify(void (*cbFn)(BLERemoteCharacteristic *, const uint8_t *, uint32_t)) {
    _cbFn = cbFn;
}

bool BLERemoteCharacteristic::canRead() {
    gatt_client_characteristic_t *_gattCharacteristic = (gatt_client_characteristic_t *)_gcc;
    return _gattCharacteristic->properties & BLERead ? true : false;
}

bool BLERemoteCharacteristic::canWrite() {
    gatt_client_characteristic_t *_gattCharacteristic = (gatt_client_characteristic_t *)_gcc;
    return _gattCharacteristic->properties & BLEWrite ? true : false;
}

bool BLERemoteCharacteristic::canWriteWithoutResponse() {
    gatt_client_characteristic_t *_gattCharacteristic = (gatt_client_characteristic_t *)_gcc;
    return _gattCharacteristic->properties & BLEWriteWithoutResponse ? true : false;
}

bool BLERemoteCharacteristic::canNotify() {
    gatt_client_characteristic_t *_gattCharacteristic = (gatt_client_characteristic_t *)_gcc;
    return _gattCharacteristic->properties & BLENotify ? true : false;
}

// Actually grab the remote data
void BLERemoteCharacteristic::pollRemote() {
    gatt_client_characteristic_t *_gattCharacteristic = (gatt_client_characteristic_t *)_gcc;
    free(_remoteValue);
    _remoteValue = nullptr;
    _remoteValueLen = 0;
    if (!canRead()) {
        return;
    }
    volatile bool waitForCB = true;
    _cbflag = &waitForCB;
    do {
        BluetoothLock lock;
        gatt_client_read_value_of_characteristic(GATTPACKETHANDLERCB(BLERemoteCharacteristic, packetHandler), con_handle, _gattCharacteristic);
    } while (0);
    _waitForCompletion(waitForCB);
}

size_t BLERemoteCharacteristic::valueLen() {
    return _remoteValueLen;
}

void *BLERemoteCharacteristic::valueData() {
    return _remoteValue;
}

String BLERemoteCharacteristic::getDescription() {
    gatt_client_characteristic_descriptor_t *d = (gatt_client_characteristic_descriptor_t *)description();
    if (!d || !con_handle) {
        DEBUGBLE("no secrription descriptor found\n");
        free(_description);
        _description = nullptr;
        return String("");
    }
    volatile bool waitForCB = true;
    _cbflag = &waitForCB;
    do {
        BluetoothLock lock;
        gatt_client_read_characteristic_descriptor(GATTPACKETHANDLERCB(BLERemoteCharacteristic, packetHandler), con_handle, d);
    } while (0);
    _waitForCompletion(waitForCB);
    String ret = String(_description);
    free(_description);
    _description = nullptr;
    return ret;
}

bool BLERemoteCharacteristic::_waitForCompletion(volatile bool &waitForCB) {
    uint32_t start = millis();
    uint32_t timeout = BLEClient::_timeout;
    while (con_handle && (millis() - start < timeout)) {
        if (!waitForCB) {
            break;
        }
        delay(50);
    }
    if (waitForCB) {
        DEBUGBLE("timeout\n");
    }
    return !waitForCB;
}

// Write date to the characteristic
bool BLERemoteCharacteristic::setValue(uint8_t *data, size_t size) {
    gatt_client_characteristic_t *_gattCharacteristic = (gatt_client_characteristic_t *)_gcc;
    if (!canWrite()) {
        return false;
    }
    if (con_handle) {
        volatile bool waitForCB = true;
        _cbflag = &waitForCB;
        // Can't BT lock this call
        gatt_client_write_value_of_characteristic(GATTPACKETHANDLERCB(BLERemoteCharacteristic, packetHandler), con_handle, _gattCharacteristic->value_handle, size, (uint8_t *)data);
        _waitForCompletion(waitForCB);
        return !waitForCB;
    } else {
        return false;
    }
}

bool BLERemoteCharacteristic::setValue(const String &value) {
    char *c = strdup(value.c_str());
    bool ret = setValue((uint8_t *)c, strlen(c));
    free(c);
    return ret;
}

bool BLERemoteCharacteristic::setValue(bool data8) {
    return setValue((uint8_t *)&data8, sizeof(data8));
}

bool BLERemoteCharacteristic::setValue(uint8_t data8) {
    return setValue((uint8_t *)&data8, sizeof(data8));
}

bool BLERemoteCharacteristic::setValue(uint16_t data16) {
    return setValue((uint8_t *)&data16, sizeof(data16));
}

bool BLERemoteCharacteristic::setValue(uint32_t data32) {
    return setValue((uint8_t *)&data32, sizeof(data32));
}

bool BLERemoteCharacteristic::setValue(int data32) {
    return setValue((uint8_t *)&data32, sizeof(data32));
}

bool BLERemoteCharacteristic::setValue(float data32) {
    return setValue((uint8_t *)&data32, sizeof(data32));
}

bool BLERemoteCharacteristic::setValue(double data64) {
    return setValue((uint8_t *)&data64, sizeof(data64));
}

bool BLERemoteCharacteristic::enableNotifications() {
    if (!canNotify()) {
        DEBUGBLE("can't notify\n");
        return false;
    }
    if (_notifyOn) {
        DEBUGBLE("notify already on\n");
        return true;
    }
    volatile bool waitForCB = true;
    _cbflag = &waitForCB;
    gatt_client_characteristic_t *_gattCharacteristic = (gatt_client_characteristic_t *)_gcc;
    gatt_client_notification_t *_gattNotification = (gatt_client_notification_t *)_gcn;
    do {
        BluetoothLock lock;
        gatt_client_write_client_characteristic_configuration(GATTPACKETHANDLERCB(BLERemoteCharacteristic, packetHandler), con_handle, _gattCharacteristic, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
    } while (0);
    _waitForCompletion(waitForCB);
    do {
        BluetoothLock lock;
        gatt_client_listen_for_characteristic_value_updates(_gattNotification, GATTPACKETHANDLERCB(BLERemoteCharacteristic, packetHandler), con_handle, _gattCharacteristic);
    } while (0);

    _notifyOn = true;
    return true;
}

void BLERemoteCharacteristic::disableNotifications() {
    gatt_client_characteristic_t *_gattCharacteristic = (gatt_client_characteristic_t *)_gcc;
    gatt_client_notification_t *_gattNotification = (gatt_client_notification_t *)_gcn;
    if (_notifyOn) {
        volatile bool waitForCB = true;
        _cbflag = &waitForCB;
        if (con_handle) {
            BluetoothLock lock;
            gatt_client_write_client_characteristic_configuration(GATTPACKETHANDLERCB(BLERemoteCharacteristic, packetHandler), con_handle, _gattCharacteristic, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NONE);
        }
        _waitForCompletion(waitForCB);
        do {
            BluetoothLock lock;
            gatt_client_stop_listening_for_characteristic_value_updates(_gattNotification);
        } while (0);
        _notifyOn = false;
    }
}

void BLERemoteCharacteristic::disconnect() {
    gatt_client_notification_t *_gattNotification = (gatt_client_notification_t *)_gcn;
    gatt_client_characteristic_t *_gattCharacteristic = (gatt_client_characteristic_t *)_gcc;
    free(_remoteValue);
    _remoteValue = nullptr;
    _remoteValueLen = 0;
    con_handle = 0;
    if (_notifyOn) {
        BluetoothLock lock;
        gatt_client_stop_listening_for_characteristic_value_updates(_gattNotification);
    }
    _gattCharacteristic->properties = 0; // No more read, write, notify
}

uint16_t BLERemoteCharacteristic::_valueHandle() {
    gatt_client_characteristic_t *_gattCharacteristic = (gatt_client_characteristic_t *)_gcc;
    return _gattCharacteristic->value_handle;
}

uint16_t BLERemoteCharacteristic::_configurationHandle() {
    gatt_client_characteristic_descriptor_t *_gattDescriptor = (gatt_client_characteristic_descriptor_t *)_gccd;
    for (uint8_t i = 0; i < _gattDescriptors; i++) {
        if (_gattDescriptor[i].uuid16 == 0x2902) {
            return _gattDescriptor[i].handle;
        }
    }
    return 0;
}

void /*gatt_client_characteristic_descriptor_t*/ *BLERemoteCharacteristic::description() {
    gatt_client_characteristic_descriptor_t *_gattDescriptor = (gatt_client_characteristic_descriptor_t *)_gccd;
    for (uint8_t i = 0; i < _gattDescriptors; i++) {
        if (_gattDescriptor[i].uuid16 == 0x2901) {
            return &_gattDescriptor[i];
        }
    }
    return nullptr;
}
