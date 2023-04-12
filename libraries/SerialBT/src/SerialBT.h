/*
    Serial-over-Bluetooth for the Raspberry Pi Pico RP2040

    Copyright (c) 2023 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include <_needsbt.h>
#include <Arduino.h>
#include <api/HardwareSerial.h>
#include <stdarg.h>
#include <queue>
#include <pico/cyw43_arch.h>
#include <CoreMutex.h>
#include <btstack.h>

class SerialBT_;
extern SerialBT_ SerialBT;

class SerialBT_ : public HardwareSerial {
public:
    SerialBT_();

    bool setFIFOSize(size_t size);

    void begin(unsigned long baud = 115200) override {
        begin(baud, SERIAL_8N1);
    };
    void begin(unsigned long baud, uint16_t config) override;
    void end() override;

    virtual int peek() override;
    virtual int read() override;
    virtual int available() override;
    virtual int availableForWrite() override;
    virtual void flush() override;
    virtual size_t write(uint8_t c) override;
    virtual size_t write(const uint8_t *p, size_t len) override;
    using Print::write;
    bool overflow();
    operator bool() override;

    // ESP8266 compat
    void setDebugOutput(bool unused) {
        (void) unused;
    }

    static void PacketHandlerWrapper(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t packet_size) {
        SerialBT.packetHandler(packet_type, channel, packet, packet_size);
    }

    void packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size);

    static void lockBluetooth() {
        async_context_acquire_lock_blocking(cyw43_arch_async_context());
    }

    static void unlockBluetooth() {
        async_context_release_lock(cyw43_arch_async_context());
    }

private:
    bool _running = false;
    mutex_t _mutex;
    bool _overflow = false;
    volatile bool _connected = false;


    // Lockless, IRQ-handled circular queue
    uint32_t _writer;
    uint32_t _reader;
    size_t   _fifoSize = 32;
    uint8_t *_queue;

    const int RFCOMM_SERVER_CHANNEL = 1;

    uint16_t _channelID;
    uint8_t  _spp_service_buffer[150];
    btstack_packet_callback_registration_t _hci_event_callback_registration;

    volatile int _writeLen = 0;
    const void *_writeBuff;
};
