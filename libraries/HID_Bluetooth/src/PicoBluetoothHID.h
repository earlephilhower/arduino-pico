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

#include <bluetooth.h>
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
#include <ble/gatt-service/hids_device.h>
#include <ble/att_db.h>
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

    bool startHID(const char *localName, const char *hidName, uint16_t hidClass, uint8_t hidSubclass, const uint8_t *hidDescriptor, uint16_t hidDescriptorSize);

private:
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
public:
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
        __lockBluetooth();
        if (connected()) {
            _needToSend = true;
            _sendReportID = id;
            _sendReport = rpt;
            _sendReportLen = len;
            hid_device_request_can_send_now_event(getCID());
        }
        __unlockBluetooth();
        while (connected() && _needToSend) {
            /* noop busy wait */
        }
        return connected();
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
