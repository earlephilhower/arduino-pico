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

#include <Arduino.h>
#include "btstack.h"
#include <list>
#include <memory>

#include "BluetoothHCI.h"
#include "BluetoothLock.h"

#define CCALLBACKNAME _BTHCICB
#include <ctocppcallback.h>


#define PACKETHANDLERCB(class, cbFcn) \
  (_BTHCICB<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__>::func = std::bind(&class::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), \
   static_cast<btstack_packet_handler_t>(_BTHCICB<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__ - 1>::callback))


void BluetoothHCI::install() {
    // Register for HCI events.
    hci_event_callback_registration.callback = PACKETHANDLERCB(BluetoothHCI, hci_packet_handler);
    hci_add_event_handler(&hci_event_callback_registration);
}

void BluetoothHCI::begin() {
    _running = true;
    hci_power_control(HCI_POWER_ON);
}

void BluetoothHCI::uninstall() {
    BluetoothLock b;
    hci_remove_event_handler(&hci_event_callback_registration);
    _running = false;
}

bool BluetoothHCI::running() {
    return _hciRunning;
}

std::list<BTDeviceInfo> BluetoothHCI::scan(uint32_t mask, int scanTimeSec, bool async) {
    _scanMask = mask;
    _btdList.clear();
    if (!_running) {
        return _btdList;
    }
    _scanning = true;
    while (!_hciRunning) {
        DEBUGV("HCI::scan(): Waiting for HCI to come up\n");
        delay(10);
    }
    int inquiryTime = (scanTimeSec * 1000) / 1280; // divide by 1.280
    if (!inquiryTime) {
        inquiryTime = 1;
    }
    DEBUGV("HCI::scan(): inquiry start\n");
    // Only need to lock around the inquiry start command, not the wait
    {
        BluetoothLock b;
        gap_inquiry_start(inquiryTime);
    }
    if (async) {
        return _btdList;
    }

    while (_scanning) {
        delay(10);
    }
    DEBUGV("HCI::scan(): inquiry end\n");
    return _btdList;
}

bool BluetoothHCI::scanAsyncDone() {
    return _scanning;
}
std::list<BTDeviceInfo> BluetoothHCI::scanAsyncResult() {
    return _btdList;
}


void BluetoothHCI::hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void)channel;
    (void)size;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    bd_addr_t address;
    uint32_t cod;
    int8_t rssi;
    const char *name;
    char name_buffer[241];

    switch (hci_event_packet_get_type(packet)) {
    case  BTSTACK_EVENT_STATE:
        _hciRunning = (btstack_event_state_get_state(packet) == HCI_STATE_WORKING);
        break;
    case HCI_EVENT_PIN_CODE_REQUEST:
        hci_event_pin_code_request_get_bd_addr(packet, address);
        gap_pin_code_response(address, "0000");
        break;
    case GAP_EVENT_INQUIRY_RESULT:
        if (!_scanning) {
            return;
        }
        gap_event_inquiry_result_get_bd_addr(packet, address);
        cod = gap_event_inquiry_result_get_class_of_device(packet);
        if (gap_event_inquiry_result_get_rssi_available(packet)) {
            rssi = gap_event_inquiry_result_get_rssi(packet);
        } else {
            rssi = -128;
        }
        if (gap_event_inquiry_result_get_name_available(packet)) {
            int name_len = gap_event_inquiry_result_get_name_len(packet);
            memcpy(name_buffer, gap_event_inquiry_result_get_name(packet), name_len);
            name_buffer[name_len] = 0;
            name = name_buffer;
        } else {
            name = "";
        }
        DEBUGV("HCI: Scan found '%s', COD 0x%08X, RSSI %d, MAC %02X:%02X:%02X:%02X:%02X:%02X\n", name, (unsigned int)cod, rssi, address[0], address[1], address[2], address[3], address[4], address[5]);
        if ((_scanMask & cod) == _scanMask) {
            // Sometimes we get multiple reports for the same MAC, so remove any old reports since newer will have newer RSSI
            bool updated = false;
            for (auto itr = _btdList.begin(); (itr != _btdList.end()) && !updated; itr++) {
                if (!memcmp(itr->address(), address, sizeof(address))) {
                    // Sometimes the name is missing on reports, so if we found it once preserve it
                    if (!name_buffer[0] && itr->name()[0]) {
                        strcpy(name_buffer, itr->name());
                    }
                    _btdList.erase(itr);
                    updated = true;
                }
            }
            BTDeviceInfo btd(cod, address, rssi, name);
            _btdList.push_back(btd);
        }
        break;
    case GAP_EVENT_INQUIRY_COMPLETE:
        _scanning = false;
        break;
    default:
        break;
    }
}
