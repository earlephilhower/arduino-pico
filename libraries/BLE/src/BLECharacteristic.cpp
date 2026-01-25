/*
    BLECharacteristic - BLE service characteristics
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
#include "BLECharacteristic.h"
#include <ble/att_db_util.h>
#include <list>
#include <BluetoothLock.h>

BLECharacteristic::BLECharacteristic(BLEUUID u) {
    _uuid = u;
    _flags = ATT_PROPERTY_READ | ATT_PROPERTY_DYNAMIC;
    _data = nullptr;
    _dataLen = 0;
    _description = nullptr;
    _descriptionLen = 0;
    _canSend = malloc(sizeof(btstack_context_callback_registration_t));
}

BLECharacteristic::BLECharacteristic(BLEUUID u, uint16_t p, const char *desc, uint8_t permR, uint8_t permW) {
    _uuid = u;
    _flags = p | ATT_PROPERTY_DYNAMIC;
    _data = nullptr;
    _dataLen = 0;
    _readPerm = permR;
    _writePerm = permW;
    _description = desc ? strdup(desc) : nullptr;
    _descriptionLen = desc ? strlen(desc) : 0;
    _canSend = malloc(sizeof(btstack_context_callback_registration_t));
}

BLECharacteristic::~BLECharacteristic() {
    free(_canSend);
    free(_description);
}

void BLECharacteristic::setCallbacks(BLECharacteristicCallbacks *cb) {
    _cb = cb;
}
void BLECharacteristic::onWrite(void (*fn)(BLECharacteristic *)) {
    _writeCB = fn;
}

void BLECharacteristic::onRead(void (*fn)(BLECharacteristic *)) {
    _readCB = fn;
}

void BLECharacteristic::setValue(const uint8_t *data, size_t size) {
    _charData = realloc(_charData, size);
    memcpy(_charData, data, size);
    _charLen = size;
    requestNotify();
}

void BLECharacteristic::setValue(const String &value) {
    _charData = realloc(_charData, value.length());
    memcpy(_charData, value.c_str(), value.length());
    _charLen = value.length();
    requestNotify();
}

// TODO - Might need to twiddle byte order?
void BLECharacteristic::setValue(uint16_t data16) {
    _charData = realloc(_charData, sizeof(data16));
    memcpy(_charData, &data16, sizeof(data16));
    _charLen = sizeof(data16);
    requestNotify();
}

// TODO - Might need to twiddle byte order?
void BLECharacteristic::setValue(uint32_t data32) {
    _charData = realloc(_charData, sizeof(data32));
    memcpy(_charData, &data32, sizeof(data32));
    _charLen = sizeof(data32);
    requestNotify();
}

// TODO - Might need to twiddle byte order?
void BLECharacteristic::setValue(int data32) {
    _charData = realloc(_charData, sizeof(data32));
    memcpy(_charData, &data32, sizeof(data32));
    _charLen = sizeof(data32);
    requestNotify();
}

// TODO - Might need to twiddle byte order?
void BLECharacteristic::setValue(float data32) {
    _charData = realloc(_charData, sizeof(data32));
    memcpy(_charData, &data32, sizeof(data32));
    _charLen = sizeof(data32);
    requestNotify();
}

// TODO - Might need to twiddle byte order?
void BLECharacteristic::setValue(double data64) {
    _charData = realloc(_charData, sizeof(data64));
    memcpy(_charData, &data64, sizeof(data64));
    _charLen = sizeof(data64);
    requestNotify();
}

size_t BLECharacteristic::valueLen() {
    return (size_t)_charLen;
}

const void *BLECharacteristic::valueData() {
    return (void *)_charData;
}

void BLECharacteristic::addATTDB() {
    if (_uuid.is16) {
        _valueHandle = att_db_util_add_characteristic_uuid16(_uuid.uuid16, _flags, _readPerm, _writePerm, (uint8_t *)_data, _dataLen);
    } else {
        _valueHandle = att_db_util_add_characteristic_uuid128(_uuid.uuid128, _flags, _readPerm, _writePerm, (uint8_t *)_data, _dataLen);
    }
    if (_flags & (ATT_PROPERTY_NOTIFY | ATT_PROPERTY_INDICATE)) {
        _configHandle = _valueHandle + 1;
        // TODO - this is a bugfix for the BTStack core.  Sets wrong flags value for these cases on the CCC.  Remove when BTStack fixed
        att_db_util_get_address()[att_db_util_get_size() - 10] |= 0x04;
    } else {
        _configHandle = 0;
    }
    if (_description) {
        // It seems the description is always writable(?)
        att_db_util_add_descriptor_uuid16(ORG_BLUETOOTH_DESCRIPTOR_GATT_CHARACTERISTIC_USER_DESCRIPTION, ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC, ATT_SECURITY_NONE, ATT_SECURITY_NONE, nullptr, 0);  //, (uint8_t *)_description, strlen(_description));
        if (_configHandle) {
            _descriptionHandle = _configHandle + 1;
        } else {
            _descriptionHandle = _valueHandle + 1;
        }
    }
}

void BLECharacteristic::setConHandle(uint16_t c) {
    con_handle = c;
}

// Called when the BLE report disconnect
void BLECharacteristic::disconnected() {
    _notificationEnabled = false;
}

void BLECharacteristic::requestNotify() {
    if (!_notificationEnabled || !con_handle) {
        return;  // Easy peasy, lemon squeezy
    }
    _notifyInfo.con_handle = con_handle;
    _notifyInfo.char_handle = _valueHandle;
    _notifyInfo.value = _charData;
    _notifyInfo.len = _charLen;
    btstack_context_callback_registration_t *cs = (btstack_context_callback_registration_t *)_canSend;
    cs->callback = _canSendCB;
    cs->context = &_notifyInfo;
    BluetoothLock lock;
    att_server_register_can_send_now_callback(cs, con_handle);
}

void BLECharacteristic::_canSendCB(void *t) {
    // Will be running in BT ctx already
    notifyInfo *n = static_cast<notifyInfo *>(t);
    att_server_notify(n->con_handle, n->char_handle, (uint8_t *)n->value, n->len);
}

int BLECharacteristic::handleRead(uint16_t attribute_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    // Will be running in BT ctx already
    if (attribute_handle == _configHandle) {
        auto ret = att_read_callback_handle_little_endian_16(_configData, offset, buffer, buffer_size);
        return ret;
    } else if (attribute_handle == _valueHandle) {
        if (_cb) {
            _cb->onRead(this);
        }
        if (_readCB) {
            _readCB(this);
        }
        if (_charData) {
            return att_read_callback_handle_blob((uint8_t *)_charData, _charLen, offset, buffer, buffer_size);
        }
    } else if ((attribute_handle == _descriptionHandle) && _description) {
        return att_read_callback_handle_blob((uint8_t *)_description, _descriptionLen, offset, buffer, buffer_size);
    }
    return 0;
}

int BLECharacteristic::handleWrite(uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    // Will be running in BT ctx already
    if (attribute_handle == _configHandle) {
        _notificationEnabled = little_endian_read_16(buffer, 0);
        _configData = little_endian_read_16(buffer, 0);
    } else if (attribute_handle == _valueHandle) {
        _charData = realloc(_charData, buffer_size + 1);
        memcpy(_charData, buffer, buffer_size);
        ((uint8_t *)_charData)[buffer_size] = 0;  // 0-terminate to make C-string life simpler
        _charLen = buffer_size;
        if (_cb) {
            _cb->onWrite(this);
        }
        if (_writeCB) {
            _writeCB(this);
        }
    } else if (attribute_handle == _descriptionHandle) {
        _description = realloc(_description, buffer_size);
        memcpy(_description, buffer, buffer_size);
        _descriptionLen = buffer_size;
    }
    return 0;
}
