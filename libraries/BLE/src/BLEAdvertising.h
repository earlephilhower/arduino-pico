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


#pragma once

#include <Arduino.h>
#include "BLEUUID.h"
#include "BLEAddress.h"

class BLEServer;
class BLEAdvertising;

class BLEAdvertisedDeviceCallbacks {
public:
    virtual void onStop(BLEServer *p) {
        printf("BLEScanCB onStop %p\n", p);
    }
    virtual void onReport(BLEServer *p, BLEAdvertising *a) {
        printf("BLEScanCB onReport %p %p\n", p, a);
    }
};

class BLEAdvertising {
public:
    BLEAdvertising();
    ~BLEAdvertising();

    void setName(const char *c);
    const char *getName();

    void setOnlyUUID(BLEUUID uuid);
    void setPartialUUID(BLEUUID uuid);
    BLEUUID getUUID();

    void setAppearance(uint16_t a);
    uint16_t getAppearance();

    void setManufacturerData(const uint8_t *data, int len);
    int getManufacturerData(const uint8_t **data);

    void setURL(const char *url);

    String toString();

    void setAdvertisingIntervalMin(uint16_t advmin);
    uint16_t getAdvertisingIntervalMin();

    void setAdvertisingIntervalMax(uint16_t advmax);
    uint16_t getAdvertisingIntervalMax();

    bool build();

    uint8_t *getBlob();
    uint8_t getBlobLen();

    void setAddress(BLEAddress addr);
    BLEAddress address();

    void setRSSI(int8_t rssi);

private:
    bool append(uint8_t c);

    uint8_t _advDataLen = 0;
    uint8_t _advData[31];  // BLE spec is 31 bytes max

    char _name[31] = {} ; // larger than can ever happen
    uint8_t _manufData[31]; // larger than is possible to specify
    int _manufDataLen = 0;
    bool _partial = false;
    BLEUUID _uuid;
    BLEAddress _addr;
    int8_t _rssi = -128;
    uint16_t _appearance = 0;
    uint16_t _advIntMin = 0x30;
    uint16_t _advIntMax = 0x30;
    char _url[31] = {};
    uint8_t _uri = 0;
};
