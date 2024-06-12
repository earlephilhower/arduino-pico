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

// Based off of the BlueKitchen HID master demo
/*
    Copyright (C) 2023 BlueKitchen GmbH

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
    4. Any redistribution, use, or modification is done solely for
      personal benefit and not for any commercial purpose or for
      monetary gain.

    THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
    ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BLUEKITCHEN
    GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

    Please inquire about commercial licensing options at
    contact@bluekitchen-gmbh.com

*/

#include <Arduino.h>
#include "btstack.h"
#include <list>
#include <memory>

#include <BluetoothLock.h>
#include "BluetoothHIDMaster.h"

#define CCALLBACKNAME _BTHIDCB
#include <ctocppcallback.h>


#define PACKETHANDLERCB(class, cbFcn) \
  (CCALLBACKNAME<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__>::func = std::bind(&class::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), \
   static_cast<btstack_packet_handler_t>(CCALLBACKNAME<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__ - 1>::callback))


void BluetoothHIDMaster::begin(bool ble, const char *bleName) {
    _ble = ble;
    if (!ble) {
        // Initialize HID Host
        hid_host_init(_hid_descriptor_storage, sizeof(_hid_descriptor_storage));
        hid_host_register_packet_handler(PACKETHANDLERCB(BluetoothHIDMaster, hid_packet_handler));
    } else {
        if (bleName) {
            _hci.setBLEName(bleName);
        }
        _hci.setPairOnMeta(true);
    }

    // Initialize L2CAP
    l2cap_init();

    // Initialize LE Security Manager. Needed for cross-transport key derivation
    if (ble) {
        // register for events from Security Manager
        _sm_event_callback_registration.callback = PACKETHANDLERCB(BluetoothHIDMaster, sm_packet_handler);
        sm_add_event_handler(&_sm_event_callback_registration);
    }
    sm_init();

    if (ble) {
        gatt_client_init();
        hids_client_init(_hid_descriptor_storage, sizeof(_hid_descriptor_storage));
    } else {
        // Allow sniff mode requests by HID device and support role switch
        gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_SNIFF_MODE | LM_LINK_POLICY_ENABLE_ROLE_SWITCH);

        // try to become master on incoming connections
        hci_set_master_slave_policy(HCI_ROLE_MASTER);
    }

    // enabled EIR
    hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);

    _hci.install();

    _running = true;
    _hci.begin();
}

void BluetoothHIDMaster::end() {
    BluetoothLock b;
    _hci.uninstall();
    _running = false;
}

bool BluetoothHIDMaster::running() {
    return _hci.running() && _hidConnected;
}

bool BluetoothHIDMaster::hciRunning() {
    return _hci.running();
}

void BluetoothHIDMaster::onMouseMove(void (*cb)(void *, int, int, int), void *cbData) {
    _mouseMoveCB = cb;
    _mouseMoveData = cbData;
}

void BluetoothHIDMaster::onMouseButton(void (*cb)(void *, int, bool), void *cbData) {
    _mouseButtonCB = cb;
    _mouseButtonData = cbData;
}

void BluetoothHIDMaster::onKeyDown(void (*cb)(void *, int), void *cbData) {
    _keyDownCB = cb;
    _keyDownData = cbData;
}

void BluetoothHIDMaster::onKeyUp(void (*cb)(void *, int), void *cbData) {
    _keyUpCB = cb;
    _keyUpData = cbData;
}

void BluetoothHIDMaster::onConsumerKeyDown(void (*cb)(void *, int), void *cbData) {
    _consumerKeyDownCB = cb;
    _consumerKeyDownData = cbData;
}

void BluetoothHIDMaster::onConsumerKeyUp(void (*cb)(void *, int), void *cbData) {
    _consumerKeyUpCB = cb;
    _consumerKeyUpData = cbData;
}

void BluetoothHIDMaster::onJoystick(void (*cb)(void *, int, int, int, int, uint8_t, uint32_t), void *cbData) {
    _joystickCB = cb;
    _joystickData = cbData;
}

std::vector<BTDeviceInfo> BluetoothHIDMaster::scan(uint32_t mask, int scanTimeSec, bool async) {
    return _hci.scan(mask, scanTimeSec, async);
}

bool BluetoothHIDMaster::scanAsyncDone() {
    return _hci.scanAsyncDone();
}
std::vector<BTDeviceInfo> BluetoothHIDMaster::scanAsyncResult() {
    return _hci.scanAsyncResult();
}

bool BluetoothHIDMaster::connected() {
    return _hidConnected && _hid_host_descriptor_available;
}

bool BluetoothHIDMaster::connect(const uint8_t *addr) {
    if (!_running || _ble) {
        return false;
    }
    while (!_hci.running()) {
        delay(10);
    }
    uint8_t a[6];
    memcpy(a, addr, sizeof(a));
    return ERROR_CODE_SUCCESS == hid_host_connect(a, HID_PROTOCOL_MODE_REPORT, &_hid_host_cid);
}

bool BluetoothHIDMaster::connectCOD(uint32_t cod) {
    if (!_running || _ble) {
        return false;
    }
    while (!_hci.running()) {
        delay(10);
    }

    uint8_t a[6];
    clearPairing();
    auto l = scan(cod);
    for (auto e : l) {
        DEBUGV("Scan connecting %s at %s ... ", e.name(), e.addressString());
        memcpy(a, e.address(), sizeof(a));
        if (ERROR_CODE_SUCCESS == hid_host_connect(a, HID_PROTOCOL_MODE_REPORT, &_hid_host_cid)) {
            DEBUGV("Connection established\n");
            return true;
        }
        DEBUGV("Failed\n");
    }
    return false;
}

bool BluetoothHIDMaster::connectBLE(const uint8_t *addr, int addrType) {
    if (!_running || !_ble) {
        return false;
    }
    while (!_hci.running()) {
        delay(10);
    }
    uint8_t a[6];
    memcpy(a, addr, sizeof(a));
    return ERROR_CODE_SUCCESS == gap_connect(a, (bd_addr_type_t)addrType);
}

bool BluetoothHIDMaster::connectBLE() {
    if (!_running || !_ble) {
        return false;
    }
    while (!_hci.running()) {
        delay(10);
    }

    clearPairing();
    auto l = _hci.scanBLE(0x1812 /* HID */);
    for (auto e : l) {
        DEBUGV("Scan connecting %s at %s ... ", e.name(), e.addressString());
        if (connectBLE(e.address(), e.addressType())) {
            DEBUGV("Connection established\n");
            return true;
        }
        DEBUGV("Failed\n");
    }
    return false;
}

bool BluetoothHIDMaster::connectKeyboard() {
    return connectCOD(0x2540);
}

bool BluetoothHIDMaster::connectMouse() {
    return connectCOD(0x2580);
}

bool BluetoothHIDMaster::connectJoystick() {
    return connectCOD(0x2508);
}

bool BluetoothHIDMaster::connectAny() {
    return connectCOD(0x2500);
}

bool BluetoothHIDMaster::disconnect() {
    BluetoothLock b;
    if (!_running || !connected()) {
        return false;
    }
    if (!_ble && connected()) {
        hid_host_disconnect(_hid_host_cid);
    } else if (_ble && connected()) {
        gap_disconnect(_hci.getHCIConn());
    }
    _hid_host_descriptor_available = false;
    return true;
}

void BluetoothHIDMaster::clearPairing() {
    BluetoothLock b;
    if (connected()) {
        if (_ble) {
            gap_disconnect(_hci.getHCIConn());
        } else {
            hid_host_disconnect(_hid_host_cid);
        }
    }
    gap_delete_all_link_keys();
    _hid_host_descriptor_available = false;
}

void BluetoothHIDMaster::hid_host_handle_interrupt_report(btstack_hid_parser_t * parser) {
    uint8_t new_keys[NUM_KEYS];
    uint8_t tosend[NUM_KEYS];
    int tosendcnt = 0;
    memset(new_keys, 0, sizeof(new_keys));
    int     new_keys_count = 0;

    uint16_t new_consumer_key = 0;
    uint32_t newMB = 0;
    bool noMB = false;

    bool updCons = false;
    bool updKey = false;
    bool updMB = false;
    bool updJoy = false;

    bool updMouse = false;
    int dx = 0;
    int dy = 0;
    int dz = 0;
    int rz = 0;
    int dwheel = 0;
    uint8_t hat = 0;

    while (btstack_hid_parser_has_more(parser)) {
        uint16_t usage_page;
        uint16_t usage;
        int32_t  value;
        btstack_hid_parser_get_field(parser, &usage_page, &usage, &value);
        if (usage_page == 0x01) {
            updMouse = true;
            updJoy = true;
            if (usage == 0x30) {
                dx = value;
            } else if (usage == 0x31) {
                dy = value;
            } else if (usage == 0x32) {
                dz = value;
            } else if (usage == 0x35) {
                rz = value;
            } else if (usage == 0x38) {
                dwheel = value;
            } else if (usage == 0x39) {
                hat = value & 0xff;
            }
        } else if (usage_page == 0x09) {
            updMB = true;
            if (usage == 0) {
                noMB = true;
            }
            if (!noMB && value && (usage > 0)) {
                newMB |= 1 << (usage - 1);
            }

        } else if (usage_page == 0x0c) {
            updCons = true;
            if (value) {
                new_consumer_key = usage;
                // check if usage was used last time (and ignore in that case)
                if (usage == last_consumer_key) {
                    usage = 0;
                }
                if (usage == 0) {
                    continue;
                }
                if (last_consumer_key) {
                    if (_consumerKeyUpCB) {
                        _consumerKeyUpCB(_consumerKeyUpData, last_consumer_key);
                    }
                }
                if (_consumerKeyDownCB) {
                    _consumerKeyDownCB(_consumerKeyDownData, usage);
                }
            } else if (last_consumer_key == usage) {
                if (_consumerKeyUpCB) {
                    _consumerKeyUpCB(_consumerKeyUpData, last_consumer_key);
                }
            }
        } else if (usage_page == 0x07) {
            updKey = true;
            if (value) {
                new_keys[new_keys_count++] = usage;
                // check if usage was used last time (and ignore in that case)
                int i;
                for (i = 0; i < NUM_KEYS; i++) {
                    if (usage == last_keys[i]) {
                        usage = 0;
                    }
                }
                if (usage == 0) {
                    continue;
                }
                tosend[tosendcnt++] = usage;
            }
        }
    }
    if (updKey) {
        bool found;
        for (int i = 0; i < NUM_KEYS; i++) {
            found = false;
            for (int j = 0; j < NUM_KEYS; j++) {
                if (last_keys[i] == new_keys[j]) {
                    found = true;
                    break;
                }
            }
            if (!found && last_keys[i] && _keyUpCB) {
                _keyUpCB(_keyUpData, last_keys[i]);
            }
        }
        for (int i = 0; _keyDownCB && i < tosendcnt; i++) {
            _keyDownCB(_keyDownData, tosend[i]);
        }
        memcpy(last_keys, new_keys, NUM_KEYS);
    }
    if (updCons) {
        last_consumer_key = new_consumer_key;
    }
    if (updMB && _mouseButtonCB)  {
        if (lastMB != newMB) {
            for (int i = 0; i < 8; i++) {
                int mask = 1 << i;
                if ((lastMB & mask) && !(newMB & mask)) {
                    _mouseButtonCB(_mouseButtonData, i, false);
                } else if (!(lastMB & mask) && (newMB & mask)) {
                    _mouseButtonCB(_mouseButtonData, i, true);
                }
            }
            lastMB = newMB;
        }
    }

    if (updMouse && _mouseMoveCB) {
        _mouseMoveCB(_mouseMoveData, dx, dy, dwheel);
    }
    if (updJoy && _joystickCB) {
        _joystickCB(_joystickData, dx, dy, dz, rz, hat, newMB);
    }
}


void BluetoothHIDMaster::hid_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void)channel;
    (void)size;

    if ((packet_type != HCI_EVENT_PACKET) || (hci_event_packet_get_type(packet) != HCI_EVENT_HID_META)) {
        return;
    }

    uint8_t   status;
    switch (hci_event_hid_meta_get_subevent_code(packet)) {

    case HID_SUBEVENT_INCOMING_CONNECTION:
        // There is an incoming connection: we can accept it or decline it.
        // The hid_host_report_mode in the hid_host_accept_connection function
        // allows the application to request a protocol mode.
        // For available protocol modes, see hid_protocol_mode_t in btstack_hid.h file.
        hid_host_accept_connection(hid_subevent_incoming_connection_get_hid_cid(packet), HID_PROTOCOL_MODE_REPORT);
        break;

    case HID_SUBEVENT_CONNECTION_OPENED:
        // The status field of this event indicates if the control and interrupt
        // connections were opened successfully.
        status = hid_subevent_connection_opened_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
            DEBUGV("Connection failed, status 0x%02x\n", status);
            _hidConnected = false;
            _hid_host_cid = 0;
            return;
        }
        _hidConnected = true;
        _hid_host_descriptor_available = false;
        _hid_host_cid = hid_subevent_connection_opened_get_hid_cid(packet);
        DEBUGV("HID Host connected.\n");
        break;

    case HID_SUBEVENT_DESCRIPTOR_AVAILABLE:
        // This event will follows HID_SUBEVENT_CONNECTION_OPENED event.
        // For incoming connections, i.e. HID Device initiating the connection,
        // the HID_SUBEVENT_DESCRIPTOR_AVAILABLE is delayed, and some HID
        // reports may be received via HID_SUBEVENT_REPORT event. It is up to
        // the application if these reports should be buffered or ignored until
        // the HID descriptor is available.
        status = hid_subevent_descriptor_available_get_status(packet);
        if (status == ERROR_CODE_SUCCESS) {
            _hid_host_descriptor_available = true;
        } else {
            DEBUGV("Cannot handle input report, HID Descriptor is not available, status 0x%02x\n", status);
        }
        break;

    case HID_SUBEVENT_REPORT:
        // Handle input report.
        if (_hid_host_descriptor_available) {
            uint16_t report_len = hid_subevent_report_get_report_len(packet);
            const uint8_t *report = hid_subevent_report_get_report(packet);
            // check if HID Input Report
            if ((report_len < 1)  || (*report != 0xa1)) {
                break;
            }
            report++;
            report_len--;;
            btstack_hid_parser_t parser;
            btstack_hid_parser_init(&parser, hid_descriptor_storage_get_descriptor_data(_hid_host_cid), hid_descriptor_storage_get_descriptor_len(_hid_host_cid), HID_REPORT_TYPE_INPUT, report, report_len);
            hid_host_handle_interrupt_report(&parser);
        }
        break;

    case HID_SUBEVENT_SET_PROTOCOL_RESPONSE:
        // For incoming connections, the library will set the protocol mode of the
        // HID Device as requested in the call to hid_host_accept_connection. The event
        // reports the result. For connections initiated by calling hid_host_connect,
        // this event will occur only if the established report mode is boot mode.
        status = hid_subevent_set_protocol_response_get_handshake_status(packet);
        if (status != HID_HANDSHAKE_PARAM_TYPE_SUCCESSFUL) {
            DEBUGV("Error set protocol, status 0x%02x\n", status);
            break;
        }
        switch ((hid_protocol_mode_t)hid_subevent_set_protocol_response_get_protocol_mode(packet)) {
        case HID_PROTOCOL_MODE_BOOT:
            DEBUGV("Protocol mode set: BOOT.\n");
            break;
        case HID_PROTOCOL_MODE_REPORT:
            DEBUGV("Protocol mode set: REPORT.\n");
            break;
        default:
            DEBUGV("Unknown protocol mode.\n");
            break;
        }
        break;

    case HID_SUBEVENT_CONNECTION_CLOSED:
        // The connection was closed.
        _hidConnected = false;
        _hid_host_cid = 0;
        _hid_host_descriptor_available = false;
        DEBUGV("HID Host disconnected.\n");
        break;

    default:
        break;
    }
}



void BluetoothHIDMaster::sm_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    switch (hci_event_packet_get_type(packet)) {
    case SM_EVENT_JUST_WORKS_REQUEST:
        DEBUGV("Just works requested\n");
        sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
        break;
    case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
        DEBUGV("Confirming numeric comparison: %" PRIu32 "\n", sm_event_numeric_comparison_request_get_passkey(packet));
        sm_numeric_comparison_confirm(sm_event_passkey_display_number_get_handle(packet));
        break;
    case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
        DEBUGV("Display Passkey: %" PRIu32 "\n", sm_event_passkey_display_number_get_passkey(packet));
        break;
    case SM_EVENT_PAIRING_COMPLETE:
        switch (sm_event_pairing_complete_get_status(packet)) {
        case ERROR_CODE_SUCCESS:
            DEBUGV("Pairing complete, success\n");
            // continue - query primary services
            DEBUGV("Search for HID service.\n");
            //app_state = W4_HID_CLIENT_CONNECTED;
            hids_client_connect(_hci.getHCIConn(), PACKETHANDLERCB(BluetoothHIDMaster, handle_gatt_client_event), HID_PROTOCOL_MODE_REPORT, &_hid_host_cid);
            break;
        case ERROR_CODE_CONNECTION_TIMEOUT:
            DEBUGV("Pairing failed, timeout\n");
            break;
        case ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION:
            DEBUGV("Pairing failed, disconnected\n");
            break;
        case ERROR_CODE_AUTHENTICATION_FAILURE:
            DEBUGV("Pairing failed, reason = %u\n", sm_event_pairing_complete_get_reason(packet));
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}


void BluetoothHIDMaster::handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    uint8_t status;
    int idx;

    if (hci_event_packet_get_type(packet) != HCI_EVENT_GATTSERVICE_META) {
        return;
    }

    switch (hci_event_gattservice_meta_get_subevent_code(packet)) {
    case GATTSERVICE_SUBEVENT_HID_SERVICE_CONNECTED:
        status = gattservice_subevent_hid_service_connected_get_status(packet);
        switch (status) {
        case ERROR_CODE_SUCCESS:
            DEBUGV("HID service client connected, found %d services\n", gattservice_subevent_hid_service_connected_get_num_instances(packet));
            _hidConnected = true;
            _hid_host_descriptor_available = true;
            break;
        default:
            DEBUGV("HID service client connection failed, status 0x%02x.\n", status);
            gap_disconnect(_hci.getHCIConn());
            //handle_outgoing_connection_error();
            break;
        }
        break;

    case GATTSERVICE_SUBEVENT_HID_REPORT:
        idx = gattservice_subevent_hid_report_get_service_index(packet);
        btstack_hid_parser_t parser;
        btstack_hid_parser_init(&parser, hids_client_descriptor_storage_get_descriptor_data(_hid_host_cid, idx), hids_client_descriptor_storage_get_descriptor_len(_hid_host_cid, idx), HID_REPORT_TYPE_INPUT, gattservice_subevent_hid_report_get_report(packet), gattservice_subevent_hid_report_get_report_len(packet));
        hid_host_handle_interrupt_report(&parser);
        //hid_handle_input_report(
        //                gattservice_subevent_hid_report_get_service_index(packet),
        //                gattservice_subevent_hid_report_get_report(packet),
        //                gattservice_subevent_hid_report_get_report_len(packet));
        break;

    default:
        break;
    }
}




// Simplified US Keyboard with Shift modifier

#define CHAR_ILLEGAL     0xff
#define CHAR_RETURN     '\n'
#define CHAR_ESCAPE      27
#define CHAR_TAB         '\t'
#define CHAR_BACKSPACE   0x7f

/**
    English (US)
*/
static const uint8_t keytable_us_none [] = {
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /*   0-3 */
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',                   /*  4-13 */
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',                   /* 14-23 */
    'u', 'v', 'w', 'x', 'y', 'z',                                       /* 24-29 */
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',                   /* 30-39 */
    CHAR_RETURN, CHAR_ESCAPE, CHAR_BACKSPACE, CHAR_TAB, ' ',            /* 40-44 */
    '-', '=', '[', ']', '\\', CHAR_ILLEGAL, ';', '\'', 0x60, ',',       /* 45-54 */
    '.', '/', CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,   /* 55-60 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 61-64 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 65-68 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 69-72 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 73-76 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 77-80 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 81-84 */
    '*', '-', '+', '\n', '1', '2', '3', '4', '5',                       /* 85-97 */
    '6', '7', '8', '9', '0', '.', 0xa7,                                 /* 97-100 */
};

static const uint8_t keytable_us_shift[] = {
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /*  0-3  */
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',                   /*  4-13 */
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',                   /* 14-23 */
    'U', 'V', 'W', 'X', 'Y', 'Z',                                       /* 24-29 */
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',                   /* 30-39 */
    CHAR_RETURN, CHAR_ESCAPE, CHAR_BACKSPACE, CHAR_TAB, ' ',            /* 40-44 */
    '_', '+', '{', '}', '|', CHAR_ILLEGAL, ':', '"', 0x7E, '<',         /* 45-54 */
    '>', '?', CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,   /* 55-60 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 61-64 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 65-68 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 69-72 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 73-76 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 77-80 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 81-84 */
    '*', '-', '+', '\n', '1', '2', '3', '4', '5',                       /* 85-97 */
    '6', '7', '8', '9', '0', '.', 0xb1,                                 /* 97-100 */
};




bool HIDKeyStream::setFIFOSize(size_t size) {
    if (!size || _running) {
        return false;
    }
    _fifoSize = size + 1; // Always 1 unused entry
    return true;
}

HIDKeyStream::HIDKeyStream() {
}

HIDKeyStream::~HIDKeyStream() {
    end();
}

void HIDKeyStream::begin() {
    if (_running) {
        end();
    }
    _queue = new uint8_t[_fifoSize];
    _writer = 0;
    _reader = 0;

    _lshift = false;
    _rshift = false;

    _holding = false;

    _running = true;
}

void HIDKeyStream::end() {
    if (!_running) {
        return;
    }
    _running = false;

    delete[] _queue;
}

int HIDKeyStream::peek() {
    if (!_running) {
        return -1;
    }
    if (_writer != _reader) {
        return _queue[_reader];
    }
    return -1;
}

int HIDKeyStream::read() {
    if (!_running) {
        return -1;
    }
    if (_writer != _reader) {
        auto ret = _queue[_reader];
        asm volatile("" ::: "memory"); // Ensure the value is read before advancing
        auto next_reader = (_reader + 1) % _fifoSize;
        asm volatile("" ::: "memory"); // Ensure the reader value is only written once, correctly
        _reader = next_reader;
        return ret;
    }
    return -1;
}

int HIDKeyStream::available() {
    if (!_running) {
        return 0;
    }
    return (_fifoSize + _writer - _reader) % _fifoSize;
}

int HIDKeyStream::availableForWrite() {
    return  2 * _fifoSize - available() - 1; // Every 2 write = 1 read buffer insertion
}

void HIDKeyStream::flush() {
    // We always send blocking
}

size_t HIDKeyStream::write(uint8_t c) {
    if (!availableForWrite()) {
        return 0;
    }
    if (_holding) {
        _holding = false;
        bool state = (bool)c;
        if (_heldKey == 0xe1) {
            _lshift = state;
            return 1;
        } else if (_heldKey == 0xe6) {
            _rshift = state;
            return 1;
        } else if (state) {
            auto ascii = (_lshift || _rshift) ? keytable_us_shift[_heldKey] : keytable_us_none[_heldKey];
            if (ascii != CHAR_ILLEGAL) {
                auto next_writer = _writer + 1;
                if (next_writer == _fifoSize) {
                    next_writer = 0;
                }
                if (next_writer != _reader) {
                    _queue[_writer] = ascii;
                    asm volatile("" ::: "memory"); // Ensure the queue is written before the written count advances
                    _writer = next_writer;
                }
            }
            return 1;
        } else {
            return 1;
        }
    } else {
        if ((c < sizeof(keytable_us_shift)) || (c == 0xe1) || (c == 0xe6)) {
            _holding = true;
            _heldKey = c;
        }
        return 1;
    }
}

size_t HIDKeyStream::write(const uint8_t *p, size_t len) {
    if (!_running || !len)  {
        return 0;
    }
    size_t cnt = 0;
    for (size_t i = 0; i < len; i++) {
        if (!write(p[len])) {
            return cnt;
        }
        cnt++;
    }
    return cnt;
}

HIDKeyStream::operator bool() {
    return _running;
}

