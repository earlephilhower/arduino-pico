/*
    PicoBluetoothHID.h - Simple wrapper for BT-HID objects like
    keyboards, mice, gamepads.
    Based off of the BTStack HID main loop
    Copyright (c) 2023 Earle F. Philhower, III.  All rights reserved.

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
#include <functional>
#include <pico/cyw43_arch.h>
#include <class/hid/hid_device.h>

// The BTStack has redefinitions of this USB enum (same values, just redefined), so hide it to allow easy compilation
#define HID_REPORT_TYPE_INPUT HID_REPORT_TYPE_INPUT_BT
#define HID_REPORT_TYPE_OUTPUT HID_REPORT_TYPE_OUTPUT_BT
#define HID_REPORT_TYPE_FEATURE HID_REPORT_TYPE_FEATURE_BT
#define hid_report_type_t hid_report_type_t_bt
#include <btstack.h>
#undef hid_report_type_t
#undef HID_REPORT_TYPE_FEATURE
#undef HID_REPORT_TYPE_OUTPUT
#undef HID_REPORT_TYPE_INPUT

class PicoBluetoothHID_;
extern PicoBluetoothHID_ PicoBluetoothHID;

class PicoBluetoothHID_ {
public:
    PicoBluetoothHID_() {
    }

    ~PicoBluetoothHID_() {
    }

    // Optional callback, not used presently
    // Usage: PicoBluetoothHID.setCanSendNowCB(std::bind(&kbd::onCanSendNow, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    typedef std::function<void(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size)> BTCallback;

    void setOpenedCB(BTCallback cb) {
        _onOpened = cb;
    }

    void setClosedCB(BTCallback cb) {
        _onClosed = cb;
    }

    void setCanSendNowCB(BTCallback cb) {
        _onCanSendNow = cb;
    }

    bool startHID(const char *localName, const char *hidName, uint16_t hidClass, uint8_t hidSubclass, const uint8_t *hidDescriptor, uint16_t hidDescriptorSize) {
        if (_running) {
            return false;
        }
        _running = true;
        // Allow finding via inquiry
        gap_discoverable_control(1);
        // Use Limited Discoverable Mode; Peripheral; Keyboard as CoD
        gap_set_class_of_device(hidClass);
        // Set local name to be identified - zeroes will be replaced by actual BD ADDR
        gap_set_local_name(localName);
        // Allow for role switch in general and sniff mode
        gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE);
        // Allow for role switch on outgoing connections - this allow HID Host to become master when we re-connect to it
        gap_set_allow_role_switch(true);

        // L2CAP
        l2cap_init();
#ifdef ENABLE_BLE
        // Initialize LE Security Manager. Needed for cross-transport key derivation
        sm_init();
#endif

        // SDP Server
        sdp_init();
        bzero(_hid_service_buffer, sizeof(_hid_service_buffer));

        const uint8_t hid_boot_device = 0;
        const uint8_t hid_virtual_cable = 0;
        const uint8_t hid_remote_wake = 1;
        const uint8_t hid_reconnect_initiate = 1;
        const uint8_t hid_normally_connectable = 1;
        // When not set to 0xffff, sniff and sniff subrating are enabled
        const uint16_t host_max_latency = 1600;
        const uint16_t host_min_timeout = 3200;

        hid_sdp_record_t hid_params = {
            hidClass, hidSubclass,
            hid_virtual_cable, hid_remote_wake,
            hid_reconnect_initiate, (bool)hid_normally_connectable,
            (bool)hid_boot_device,
            host_max_latency, host_min_timeout,
            3200,
            hidDescriptor,
            hidDescriptorSize,
            hidName
        };

        hid_create_sdp_record(_hid_service_buffer, 0x10001, &hid_params);
        sdp_register_service(_hid_service_buffer);

        // See https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers if you don't have a USB Vendor ID and need a Bluetooth Vendor ID
        // device info: BlueKitchen GmbH, product 1, version 1
        device_id_create_sdp_record(_device_id_sdp_service_buffer, 0x10003, DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH, BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);
        sdp_register_service(_device_id_sdp_service_buffer);

        // HID Device
        hid_device_init(hid_boot_device, hidDescriptorSize, hidDescriptor);

        // register for HCI events
        _hci_event_callback_registration.callback = PacketHandlerWrapper;
        hci_add_event_handler(&_hci_event_callback_registration);

        // register for HID events
        hid_device_register_packet_handler(PacketHandlerWrapper);

        hci_power_control(HCI_POWER_ON);
        return true;
    }

    static void PacketHandlerWrapper(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t packet_size) {
        PicoBluetoothHID.packetHandler(packet_type, channel, packet, packet_size);
    }

    void packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size) {
        uint8_t status;
        if (type != HCI_EVENT_PACKET) {
            return;
        }
        switch (hci_event_packet_get_type(packet)) {
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
                return;
            }
            _appState = APP_NOT_CONNECTED;
            break;

        case HCI_EVENT_USER_CONFIRMATION_REQUEST:
            // ssp: inform about user confirmation request
            //log_info("SSP User Confirmation Request with numeric value '%06" PRIu32 "'\n", hci_event_user_confirmation_request_get_numeric_value(packet));
            //log_info("SSP User Confirmation Auto accept\n");
            break;

        case HCI_EVENT_HID_META:
            switch (hci_event_hid_meta_get_subevent_code(packet)) {
            case HID_SUBEVENT_CONNECTION_OPENED:
                status = hid_subevent_connection_opened_get_status(packet);
                if (status != ERROR_CODE_SUCCESS) {
                    // outgoing connection failed
                    _appState = APP_NOT_CONNECTED;
                    _hid_cid = 0;
                    return;
                }
                _appState = APP_CONNECTED;
                _hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
                if (_onOpened) {
                    _onOpened(type, channel, packet, size);
                }
                break;
            case HID_SUBEVENT_CONNECTION_CLOSED:
                _appState = APP_NOT_CONNECTED;
                _hid_cid = 0;
                if (_onClosed) {
                    _onClosed(type, channel, packet, size);
                }
                break;
            case HID_SUBEVENT_CAN_SEND_NOW:
                uint8_t report[2 + _sendReportLen];
                report[0] = 0xa1;
                report[1] = _sendReportID;
                memcpy(report + 2, _sendReport, _sendReportLen);
                hid_device_send_interrupt_message(getCID(), report, sizeof(report));
                _needToSend = false;
                if (_onCanSendNow) {
                    _onCanSendNow(type, channel, packet, size);
                }
                break;
            }
        }
    }

    bool end() {
        if (_running) {
            hci_power_control(HCI_POWER_OFF);
        }
        _running = false;
        _needToSend = false;
        return true;
    }

    bool connected() {
        return _appState == APP_CONNECTED;
    }

    bool send(int id, void *rpt, int len) {
        _needToSend = true;
        _sendReportID = id;
        _sendReport = rpt;
        _sendReportLen = len;
        lockBluetooth();
        hid_device_request_can_send_now_event(getCID());
        unlockBluetooth();
        while (connected() && _needToSend) {
            /* noop busy wait */
        }
        return connected();
    }

    static void lockBluetooth() {
        async_context_acquire_lock_blocking(cyw43_arch_async_context());
    }

    static void unlockBluetooth() {
        async_context_release_lock(cyw43_arch_async_context());
    }

    uint16_t getCID() {
        return _hid_cid;
    }

private:
    bool _running = false;

    enum {APP_BOOTING,  APP_NOT_CONNECTED, APP_CONNECTING, APP_CONNECTED} _appState = APP_BOOTING;

    BTCallback _onOpened = nullptr;
    BTCallback _onClosed = nullptr;
    BTCallback _onCanSendNow = nullptr;

    uint8_t _hid_service_buffer[300];
    uint8_t _device_id_sdp_service_buffer[100];
    btstack_packet_callback_registration_t _hci_event_callback_registration;
    uint16_t _hid_cid;

    volatile bool _needToSend = false;
    int _sendReportID;
    void *_sendReport;
    int _sendReportLen;
};
