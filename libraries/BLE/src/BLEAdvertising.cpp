/*
    BLEAdvertising - Handles advertising and discovered devices
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
#include "BLEAdvertising.h"
#include <btstack.h>

BLEAdvertising::BLEAdvertising() {
}

BLEAdvertising::~BLEAdvertising() {
}

void BLEAdvertising::setName(const char *c) {
    strcpy(_name, c);
}

const char *BLEAdvertising::getName() {
    return _name;
}

void BLEAdvertising::setOnlyUUID(BLEUUID uuid) {
    _uuid = uuid;
    _partial = false;
}

void BLEAdvertising::setPartialUUID(BLEUUID uuid) {
    _uuid = uuid;
    _partial = true;
}

BLEUUID BLEAdvertising::getUUID() {
    return _uuid;
}

void BLEAdvertising::setAppearance(uint16_t a) {
    _appearance = a;
}

uint16_t BLEAdvertising::getAppearance() {
    return _appearance;
}

void BLEAdvertising::setManufacturerData(const uint8_t *data, int len) {
    memcpy(_manufData, data, len);
    _manufDataLen = len;
}

int BLEAdvertising::getManufacturerData(const uint8_t **data) {
    *data = _manufData;
    return _manufDataLen;
}

void BLEAdvertising::setURL(const char *url) {
    if (!strncmp("https:", url, 6)) {
        _uri = 0x17;
        strcpy(_url, url + 6);
    } else {
        _uri = 0x16;
        if (!strncmp("http:", url, 5)) {
            strcpy(_url, url + 5);
        } else {
            strcpy(_url, url);
        }
    }
}

String BLEAdvertising::toString() {
    String o = String();
    o = "MAC=";
    o += _addr.toString();
    o += String(";");
    if (_name[0]) {
        o += String("Name=");
        o += String(_name);
        o += String(";");
    }
    if (_uuid) {
        o += String("UUID=");
        o += _uuid.toString();
        o += String(";");
    }
    if (_appearance) {
        char b[5];
        sprintf(b, "%04x", _appearance);
        o += String("Appearance=");
        o += String(b);
        o += String(";");
    }
    if (_url[0]) {
        o += "URL:";
        o += String(_uri == 0x17 ? "https://" : "http://");
        o += String(_url);
        o += String(";");
    }
    if (_manufDataLen) {
        o += "ManufData=";
        for (int i = 0; i < _manufDataLen; i++) {
            char b[3];
            sprintf(b, "%02X", _manufData[i]);
            o += String(b);
        }
        o += ";";
    }
    o += String("RSSI:");
    o += String(_rssi);
    o += String(";");
    return o;
}

void BLEAdvertising::setAdvertisingIntervalMin(uint16_t advmin) {
    _advIntMin = advmin;
}

uint16_t BLEAdvertising::getAdvertisingIntervalMin() {
    return _advIntMin;
}

void BLEAdvertising::setAdvertisingIntervalMax(uint16_t advmax) {
    _advIntMax = advmax;
}

uint16_t BLEAdvertising::getAdvertisingIntervalMax() {
    return _advIntMax;
}

bool BLEAdvertising::build() {
    bool ok = true;
    constexpr const int APP_AD_FLAGS = 0x06;

    _advDataLen = 0;
    ok = ok && append(2);
    ok = ok && append(BLUETOOTH_DATA_TYPE_FLAGS);
    ok = ok && append(APP_AD_FLAGS);
    if (_uuid) {
        if (_uuid.is16) {
            ok = ok && append(3);
            ok = ok && append(_partial ? BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS : BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS);
            ok = ok && append(_uuid.uuid16 & 0xff);
            ok = ok && append(_uuid.uuid16 >> 8);
        } else {
            ok = ok && append(17);
            ok = ok && append(_partial ? BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS : BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS);
            for (int i = 15; i >= 0; i--) {
                ok = ok && append(_uuid.uuid128[i]);
            }
        }
    }
    if (_appearance) {
        ok = ok && append(3);
        ok = ok && append(BLUETOOTH_DATA_TYPE_APPEARANCE);
        ok = ok && append(_appearance & 0xff);
        ok = ok && append(_appearance >> 8);
    }
    if (_manufDataLen) {
        ok = ok && append(_manufDataLen + 1);
        ok = ok && append(BLUETOOTH_DATA_TYPE_MANUFACTURER_SPECIFIC_DATA);
        for (int i = 0; i < _manufDataLen; i++) {
            ok = ok && append(_manufData[i]);
        }
    }
    if (_url[0]) {
        ok = ok && append(2 + strlen(_url));
        ok = ok && append(BLUETOOTH_DATA_TYPE_URI);
        ok = ok && append(_uri);
        for (size_t i = 0; i < strlen(_url); i++) {
            ok = ok && append(_url[i]);
        }
    }
    if (_name[0]) {
        int neededlen = 2 + strlen(_name);
        int available = 31 - _advDataLen;
        if (available >= neededlen) {
            ok = ok && append(neededlen - 1);
            ok = ok && append(BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME);
            for (size_t i = 0; i < strlen(_name); i++) {
                ok = ok && append(_name[i]);
            }
        } else {
            if (available >= 3) {  // At least 1 character name space
                ok = ok && append(available - 1);
                ok = ok && append(BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME);
                const char *p = _name;
                while (append(*p++)) { /* noop */
                }
            }
        }
    }
    hexdump(_advData, _advDataLen, 16);

    return ok;
}

uint8_t *BLEAdvertising::getBlob() {
    return _advData;
}

uint8_t BLEAdvertising::getBlobLen() {
    return _advDataLen;
}

void BLEAdvertising::setAddress(BLEAddress addr) {
    _addr = addr;
}

BLEAddress BLEAdvertising::address() {
    return BLEAddress(_addr);
}

void BLEAdvertising::setRSSI(int8_t rssi) {
    _rssi = rssi;
}

bool BLEAdvertising::append(uint8_t c) {
    if (_advDataLen < 31) {
        _advData[_advDataLen++] = c;
        return true;
    }
    return false;
}
