/*
    Bluetooth HID Master class, can connect to keyboards, mice, and joysticks
    Works with Bluetooth Classic and BLE devices

    Copyright (c) 2024 Earle F. Philhower, III <earlephilhower@yahoo.com>

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
#include <list>
#include <memory>

#include <BluetoothHCI.h>
#include <btstack.h>


// Write raw key up/down events, read ASCII chars out
class HIDKeyStream : public Stream {
public:
    HIDKeyStream();
    ~HIDKeyStream();

    bool setFIFOSize(size_t size);

    void begin();
    void end();

    virtual int peek() override;
    virtual int read() override;
    virtual int available() override;
    virtual int availableForWrite() override;
    virtual void flush() override;
    virtual size_t write(uint8_t c) override;
    virtual size_t write(const uint8_t *p, size_t len) override;
    using Print::write;
    operator bool();

private:
    bool _lshift = false;
    bool _rshift = false;
    bool _running = false;
    bool _holding = false;
    uint8_t _heldKey;

    // Lockless, IRQ-handled circular queue
    uint32_t _writer;
    uint32_t _reader;
    size_t   _fifoSize = 32;
    uint8_t *_queue;
};

class BluetoothHIDMaster {
public:
    void begin(const char *bleName) {
        begin(true, bleName);
    }
    void begin(bool ble = false, const char *bleName = nullptr);
    bool connected();
    void end();
    bool hciRunning();
    bool running();

    static const uint32_t keyboard_cod = 0x2540;
    static const uint32_t mouse_cod = 0x2540;
    static const uint32_t joystick_cod = 0x2508;
    static const uint32_t any_cod = 0;
    std::vector<BTDeviceInfo> scan(uint32_t mask, int scanTimeSec = 5, bool async = false);
    bool scanAsyncDone();
    std::vector<BTDeviceInfo> scanAsyncResult();

    bool connect(const uint8_t *addr);
    bool connectKeyboard();
    bool connectMouse();
    bool connectJoystick();
    bool connectAny();

    bool connectBLE(const uint8_t *addr, int addrType);
    bool connectBLE();

    bool disconnect();
    void clearPairing();

    void onMouseMove(void (*)(void *, int, int, int), void *cbData = nullptr);
    void onMouseButton(void (*)(void *, int, bool), void *cbData = nullptr);
    void onKeyDown(void (*)(void *, int), void *cbData = nullptr);
    void onKeyUp(void (*)(void *, int), void *cbData = nullptr);
    void onConsumerKeyDown(void (*)(void *, int), void *cbData = nullptr);
    void onConsumerKeyUp(void (*)(void *, int), void *cbData = nullptr);
    void onJoystick(void (*)(void *, int, int, int, int, uint8_t, uint32_t), void *cbData = nullptr);

private:
    bool _ble = false;
    bool connectCOD(uint32_t cod);
    BluetoothHCI _hci;
    void hid_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    uint8_t lastMB = 0;
    enum { NUM_KEYS = 6 };
    uint8_t last_keys[NUM_KEYS] = { 0 };
    uint16_t last_consumer_key = 0;
    void hid_host_handle_interrupt_report(btstack_hid_parser_t * parser);
    bool _running = false;
    volatile bool _hidConnected = false;
    uint16_t _hid_host_cid = 0;
    bool _hid_host_descriptor_available = false;
    uint8_t _hid_descriptor_storage[300];

    void (*_mouseMoveCB)(void *, int, int, int) = nullptr;
    void *_mouseMoveData;
    void (*_mouseButtonCB)(void *, int, bool) = nullptr;
    void *_mouseButtonData;

    void (*_keyDownCB)(void *, int) = nullptr;
    void *_keyDownData;
    void (*_keyUpCB)(void *, int) = nullptr;
    void *_keyUpData;

    void (*_consumerKeyDownCB)(void *, int) = nullptr;
    void *_consumerKeyDownData;
    void (*_consumerKeyUpCB)(void *, int) = nullptr;
    void *_consumerKeyUpData;

    void (*_joystickCB)(void *, int, int, int, int, uint8_t, uint32_t) = nullptr;
    void *_joystickData;


    btstack_packet_callback_registration_t _sm_event_callback_registration;
    void sm_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    void handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
};
