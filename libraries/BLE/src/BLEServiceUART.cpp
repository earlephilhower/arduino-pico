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



#include <Arduino.h>
#include <btstack.h>
#include "BLEServiceUART.h"
#include <ble/att_db_util.h>
#include <pico/async_context.h>
#include <pico/cyw43_arch.h>
#include <list>
#include <LocklessQueue.h>
#include <BluetoothLock.h>

BLEServiceUART::BLEServiceUART(int rxbuff, int txbuff)
    : BLEService(BLEUUID(SERVICE_UUID)) {
    _rx = new BLECharacteristic(BLEUUID(CHARACTERISTIC_UUID_RX), BLEWrite, "PicoW RX Port");
    _rx->setCallbacks(this);
    _tx = new BLECharacteristic(BLEUUID(CHARACTERISTIC_UUID_TX), BLERead | BLENotify, "PicoW TX Port");
    _tx->setCallbacks(this);
    addCharacteristic(_rx);
    addCharacteristic(_tx);
    _rxQueue = new LocklessQueue<uint8_t>(rxbuff);
    _txQueue = new LocklessQueue<uint8_t>(txbuff);
    _overflow = false;
    _txSize = txbuff;
    _flushAlarm = -1;
    _flushTimeout = 0;
    _lastFlush = millis();
}

void BLEServiceUART::begin() {
}

void BLEServiceUART::begin(unsigned long baud) {
    (void)baud;
    begin();
}

void BLEServiceUART::begin(unsigned long baud, uint16_t cfg) {
    (void)baud;
    (void)cfg;
    begin();
}

void BLEServiceUART::end() {
    panic("unsupported BLEUart::end");
}

void BLEServiceUART::setAutoflush(uint32_t ms) {
    _flushTimeout = ms;
}

BLEServiceUART::operator bool() {
    if (con_handle != 0) {
        return true;
    } else {
        return false;
    }
}

int BLEServiceUART::read() {
    uint8_t ret;
    if (_rxQueue->read(&ret)) {
        return ret;
    } else {
        return -1;
    }
}

int BLEServiceUART::peek() {
    uint8_t ret;
    if (_rxQueue->peek(&ret)) {
        return ret;
    } else {
        return -1;
    }
}

int BLEServiceUART::available() {
    return _rxQueue->available();
}

bool BLEServiceUART::overflow() {
    __lockBluetooth();
    bool ovf = _overflow;
    _overflow = false;
    __unlockBluetooth();
    return ovf;
}

size_t BLEServiceUART::write(uint8_t c) {
    if (_txQueue->write(c)) {
        // We can just buffer it up for now, but set reminder alarm
        noInterrupts();
        if ((_flushTimeout > 0) && (_flushAlarm < 0)) {
            _flushwork.do_work = _flushWorkCB;
            _flushwork.user_data = (void *)this;
            async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &_flushwork, _flushTimeout);
            _flushAlarm = 1;
            //_flushAlarm = add_alarm_in_ms(_flushTimeout, _flushcb, (void *)this, true);
        }
        interrupts();
        return 1;
    }
    // Write buffer is full, update characteristic
    flush();
    return _txQueue->write(c);
}

void BLEServiceUART::flush() {
    async_context_remove_at_time_worker(cyw43_arch_async_context(), &_flushwork);

    _flushAlarm = -1;
    uint8_t b[_txSize];
    size_t len = 0;
    while (len < _txSize) {
        uint8_t r;
        if (!_txQueue->read(&r)) {
            break;
        }
        b[len++] = r;
    }
    _tx->setValue(b, len);
    _lastFlush = millis();
}

// The host has sent us data...
void BLEServiceUART::onWrite(BLECharacteristic *c) {
    if (c != _rx) {
        return;  // Shouldn't ever happen
    }
    auto len = c->valueLen();
    const char *data = (const char *)c->valueData();
    for (size_t off = 0; off < len; off++) {
        if (!_rxQueue->write(data[off])) {
            _overflow = true;
        }
    }
}
