/*
    Bluetooth HCI packet handler class

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

#include <Arduino.h>
#include <list>
#include <memory>

#include "BluetoothDevice.h"
#include <btstack.h>


class BluetoothHCI_ {
public:
    void install();
    void begin();
    void uninstall();
    bool running();

    static const uint32_t speaker_cod = 0x200000 | 0x040000 | 0x000400;  // Service Class: Rendering | Audio, Major Device Class: Audio
    static const uint32_t any_cod = 0;
    std::list<BTDeviceInfo> scan(uint32_t mask, int scanTimeSec = 5, bool async = false);
    bool scanAsyncDone();
    std::list<BTDeviceInfo> scanAsyncResult();

private:
    void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    btstack_packet_callback_registration_t hci_event_callback_registration;
    volatile bool _hciRunning = false;
    uint32_t _scanMask;
    std::list<BTDeviceInfo> _btdList;
    volatile bool _scanning = false;
    bool _running = false;
};
