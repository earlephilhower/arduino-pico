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
#include "BLEUUID.h"
#include "BLEDebug.h"

class BLERemoteCharacteristic;


class BLERemoteCharacteristicCallbacks {
public:
    BLERemoteCharacteristicCallbacks() {
    }

    virtual ~BLERemoteCharacteristicCallbacks() {
    }

    virtual void onNotify(BLERemoteCharacteristic *p, const uint8_t *data, uint32_t len) {
        DEBUGBLE("onNotify %p\n", p);
    }
};


class BLERemoteCharacteristic {
public:
    ~BLERemoteCharacteristic();

    // We canna copy this object, cap'n.  Handles/etc. for btstack are singletons
    BLERemoteCharacteristic(const BLERemoteCharacteristic& x) = delete;
    BLERemoteCharacteristic operator=(const BLERemoteCharacteristic& x) = delete;


    void setCallbacks(BLERemoteCharacteristicCallbacks *cb);


    void onNotify(void (*cbFn)(BLERemoteCharacteristic *, const uint8_t *, uint32_t));

    bool canRead();
    bool canWrite();
    bool canWriteWithoutResponse();
    bool canNotify();

    size_t valueLen();
    void *valueData();

    String description();


    // Write date to the characteristic
    bool setValue(uint8_t *data, size_t size);
    bool setValue(const String &value);
    bool setValue(bool data8);
    bool setValue(uint8_t data8);
    bool setValue(uint16_t data16);
    bool setValue(uint32_t data32);
    bool setValue(int data32);
    bool setValue(float data32);
    bool setValue(double data64);

    bool enableNotifications();
    void disableNotifications();


    template<typename T>
    T get() {
        pollRemote();
        if (valueLen() >= sizeof(T)) {
            return *static_cast<const T *>(valueData());
        }
        return 0;
    }

    bool getBool() {
        return get<bool>();
    }

    char getChar() {
        return get<char>();
    }

    uint8_t getInt8() {
        return get<int8_t>();
    }

    unsigned char getUChar() {
        return get<unsigned char>();
    }

    uint8_t getUInt8() {
        return get<uint8_t>();
    }

    short getShort() {
        return get<short>();
    }

    int16_t getInt16() {
        return get<int16_t>();
    }

    unsigned short getUShort() {
        return get<unsigned short>();
    }

    uint16_t getUInt16() {
        return get<uint16_t>();
    }

    int getInt() {
        return get<int>();
    }

    int32_t getInt32() {
        return get<int32_t>();
    }

    unsigned int getUInt() {
        return get<unsigned int>();
    }

    uint32_t getUInt32() {
        return get<uint32_t>();
    }

    long getLong() {
        return get<long>();
    }

    unsigned long getULong() {
        return get<unsigned long>();
    }

    long long getLongLong() {
        return get<long long>();
    }

    int64_t getInt64() {
        return get<int64_t>();
    }

    unsigned long long getULongLong() {
        return get<unsigned long long>();
    }

    uint64_t getUInt64() {
        return get<uint64_t>();
    }

    float getFloat() {
        return get<float>();
    }

    float getDouble() {
        return get<double>();
    }

    String getString() {
        pollRemote();
        if (valueLen()) {
            return String((const char *)valueData(), valueLen());
        }
        return String("");
    }

protected:
    friend class BLEClient;
    friend class BLERemoteService;

    // Only BLERemoteService can make an object like this
    BLERemoteCharacteristic(uint16_t con, const void /*gatt_client_characteristic_t*/ *ch);

    // Actually grab the remote data
    void pollRemote();

    void disconnect();

    void scanDescriptors(volatile bool *flag);
    void packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size);
    bool _waitForCompletion(volatile bool &waitForCB);
    void /*gatt_client_characteristic_descriptor_t*/ *gattDescription();

    uint16_t _valueHandle();
    uint16_t _configurationHandle();

    volatile bool *_cbflag = nullptr;
    BLERemoteCharacteristicCallbacks *_charCB = nullptr;
    char *_description = nullptr;
    bool _notifyOn = false;
    void *_remoteValue = nullptr;
    int _remoteValueLen = 0;

    BLEUUID _uuid;
    volatile uint16_t con_handle = 0;
    uint8_t _gattDescriptors = 0;
    void (*_cbFn)(BLERemoteCharacteristic *, const uint8_t *data, uint32_t len) = nullptr;

    // Use blobs to avid including btstack.h because it breaks TinyUSB compilation in many cases
    void *_gcc = nullptr;     //gatt_client_characteristic_t _gattCharacteristic = {};
    void *_gccd = nullptr;     //gatt_client_characteristic_descriptor_t _gattDescriptor[4] = {};
    void *_gcn = nullptr;    //gatt_client_notification_t _gattNotification = {};
};

typedef std::list<BLERemoteCharacteristic *> BLERemoteCharacteristicList;
