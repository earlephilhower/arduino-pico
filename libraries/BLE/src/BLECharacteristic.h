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


#pragma once

#include <Arduino.h>
#include "BLEUUID.h"

// These are a bitflag which can be |'d together for characteristicPermission
enum BLECharacteristicPermission {
    BLEBroadcast = 1 /*ATT_PROPERTY_BROADCAST*/,
    BLERead = 2 /*ATT_PROPERTY_READ*/,
    BLEWriteWithoutResponse = 4 /*ATT_PROPERTY_WRITE_WITHOUT_RESPONSE*/,
    BLEWrite = 8 /*ATT_PROPERTY_WRITE*/,
    BLENotify = 16 /*ATT_PROPERTY_NOTIFY*/,
    BLEIndicate = 32 /*ATT_PROPERTY_INDICATE*/,
};


class BLEServer;
class BLEService;
class BLECharacteristic;


class BLECharacteristicCallbacks {
public:
    BLECharacteristicCallbacks() {
    }

    virtual ~BLECharacteristicCallbacks() {
    }

    virtual void onRead(BLECharacteristic *pCharacteristic) {
        printf("onRead %p\n", pCharacteristic);
    }

    virtual void onWrite(BLECharacteristic *pCharacteristic) {
        printf("onWrite %p\n", pCharacteristic);
    }
};

class BLECharacteristic {
public:
    BLECharacteristic(BLEUUID u);
    BLECharacteristic(BLEUUID u, uint16_t characteristicPermission, const char *desc = nullptr, uint8_t permR = 0, uint8_t permW = 0);
    BLECharacteristic(BLEUUID u, uint16_t characteristicPermission, void *data, int dataLen, const char *desc = nullptr, uint8_t permR = 0, uint8_t permW = 0);
    virtual ~BLECharacteristic();

    // We canna copy this object, cap'n.  Handles/etc. for btstack are singletons
    BLECharacteristic(const BLECharacteristic& x) = delete;
    BLECharacteristic operator=(const BLECharacteristic& x) = delete;

    void setCallbacks(BLECharacteristicCallbacks *cb);
    void onWrite(void (*fn)(BLECharacteristic *));
    void onRead(void (*fn)(BLECharacteristic *));

    void setValue(const uint8_t *data, size_t size);
    void setValue(const String &value);
    void setValue(uint16_t data16);
    void setValue(uint32_t data32);
    void setValue(int data32);
    void setValue(float data32);
    void setValue(double data64);
    template<typename T>
    T get() {
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
        if (valueLen()) {
            return String((const char *)valueData(), valueLen());
        }
        return String("");
    }

    size_t valueLen();
    const void *valueData();


protected:
    friend class BLEService;

    void addATTDB();
    virtual void setConHandle(uint16_t c);
    virtual void disconnected();

    // Notify needs a can-send call followed by a notify call, so pass all info we need in a private ctx struct
    typedef struct notifyInfo {
        uint16_t con_handle;
        uint16_t char_handle;
        void *value;
        int len;
    } notifyInfo;

    notifyInfo _notifyInfo;
    /*btstack_context_callback_registration_t *_canSend;*/
    void *_canSend = nullptr;

    void requestNotify();

    static void _canSendCB(void *t);

    int handleRead(uint16_t attribute_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
    int handleWrite(uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

    BLEUUID _uuid;
    uint16_t _flags = 0;
    uint16_t _valueHandle = 0;
    uint16_t _configHandle = 0;
    uint16_t _configData = 0;
    uint16_t _descriptionHandle = 0;
    uint8_t _readPerm = 0;
    uint8_t _writePerm = 0;
    uint16_t con_handle = 0;

    void *_description = nullptr;
    size_t _descriptionLen;

    bool _notificationEnabled = false;

    BLECharacteristicCallbacks *_cb = nullptr;
    void (*_writeCB)(BLECharacteristic *) = nullptr;
    void (*_readCB)(BLECharacteristic *) = nullptr;

    void *_data = nullptr;
    size_t _dataLen = 0;
    void *_charData = nullptr;
    size_t _charLen = 0;
};
