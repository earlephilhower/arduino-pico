/*
    BLEServiceUART - Implements simpleNordic BLE SPP with auto-flush timeout
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
#include "BLEService.h"
#include "BLECharacteristic.h"
#include <pico/async_context.h>
#include <LocklessQueue.h>

class BLEServiceUART : public BLEService, public BLECharacteristicCallbacks, public arduino::HardwareSerial {
public:
    static constexpr const char *SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    static constexpr const char *CHARACTERISTIC_UUID_RX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
    static constexpr const char *CHARACTERISTIC_UUID_TX = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

    BLEServiceUART(int rxbuff = 32, int txbuff = 32);

    void begin();
    void begin(unsigned long baud) override;
    void begin(unsigned long baud, uint16_t cfg) override;

    void end();

    void setAutoflush(uint32_t ms = 50);

    operator bool();

    int read();
    int peek();
    int available();
    bool overflow();

    size_t write(uint8_t c);

    void flush();

    using Print::write;

private:
    async_at_time_worker_t _flushwork;
    static void _flushWorkCB(async_context_t *context, struct async_work_on_timeout *timeout) {
        ((BLEServiceUART *)(timeout->user_data))->flush();
    }

    static int64_t _flushcb(alarm_id_t t, void *data) {
        (void)t;
        ((BLEServiceUART *)data)->flush();
        return 0;
    }

    // The host has sent us data...
    void onWrite(BLECharacteristic *c) override;

    BLECharacteristic *_rx;
    BLECharacteristic *_tx;
    LocklessQueue<uint8_t> *_rxQueue;
    bool _overflow;
    LocklessQueue<uint8_t> *_txQueue;
    size_t _txSize;

    alarm_id_t _flushAlarm;  // Current (if any) alarm
    uint32_t _lastFlush;     // Time we last flushed data
    uint32_t _flushTimeout;  // ms to sit on a buffer before forcing a flush on a callback
};
