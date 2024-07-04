/*
    PicoBluetoothBLEHID.h - Simple wrapper for BT-HID objects like
    keyboards, mice, gamepads using Bluetooth LE mode
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

#include <sdkoverride/bluetooth.h>
#include <_needsbt.h>
#include <Arduino.h>
#include <functional>
#include <pico/cyw43_arch.h>
#include <class/hid/hid_device.h>

#include "HID_Bluetooth.h"

// The BTStack has redefinitions of this USB enum (same values, just redefined), so hide it to allow easy compilation
#define HID_REPORT_TYPE_INPUT HID_REPORT_TYPE_INPUT_BT
#define HID_REPORT_TYPE_OUTPUT HID_REPORT_TYPE_OUTPUT_BT
#define HID_REPORT_TYPE_FEATURE HID_REPORT_TYPE_FEATURE_BT
#define hid_report_type_t hid_report_type_t_bt
#include <sdkoverride/hids_device.h>
#include <sdkoverride/att_db.h>
#include <btstack.h>
#undef hid_report_type_t
#undef HID_REPORT_TYPE_FEATURE
#undef HID_REPORT_TYPE_OUTPUT
#undef HID_REPORT_TYPE_INPUT

#include <ble/sm.h>
#include <ble/att_server.h>
#include <btstack_event.h>
#include <ble/gatt-service/battery_service_server.h>
#include <ble/gatt-service/device_information_service_server.h>
//#include <ble/gatt-service/hids_device.h>



class PicoBluetoothBLEHID_;
extern PicoBluetoothBLEHID_ PicoBluetoothBLEHID;

class PicoBluetoothBLEHID_ {
public:
    PicoBluetoothBLEHID_() {
    }

    ~PicoBluetoothBLEHID_() {
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

    bool startHID(const char *localName, const char *hidName, uint16_t appearance, const uint8_t *hidDescriptor, uint16_t hidDescriptorSize, int battery = 100);

private:
    void packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size) {
        uint8_t result;
        uint8_t reportID;

        if (type != HCI_EVENT_PACKET) {
            return;
        }
        switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            _con_handle = HCI_CON_HANDLE_INVALID;
            if (_onClosed) {
                _onClosed(type, channel, packet, size);
            }
            break;
        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;
        case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
            // printf("Confirming numeric comparison: %"PRIu32"\n", sm_event_numeric_comparison_request_get_passkey(packet));
            sm_numeric_comparison_confirm(sm_event_passkey_display_number_get_handle(packet));
            break;
        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            //printf("Display Passkey: %"PRIu32"\n", sm_event_passkey_display_number_get_passkey(packet));
            break;
        case HCI_EVENT_HIDS_META:
            switch (hci_event_hids_meta_get_subevent_code(packet)) {
            case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
                _con_handle = hids_subevent_input_report_enable_get_con_handle(packet);
                if (_onOpened) {
                    _onOpened(type, channel, packet, size);
                }
                break;
            case HIDS_SUBEVENT_BOOT_KEYBOARD_INPUT_REPORT_ENABLE:
                _con_handle = hids_subevent_boot_keyboard_input_report_enable_get_con_handle(packet);
                if (_onOpened) {
                    _onOpened(type, channel, packet, size);
                }
                break;
            case HIDS_SUBEVENT_PROTOCOL_MODE:
                _protocol_mode = hids_subevent_protocol_mode_get_protocol_mode(packet);
                break;
            case HIDS_SUBEVENT_CAN_SEND_NOW:
                switch (_protocol_mode) {
                case 0:
                    //We cannot distinguish between kbd & mouse in boot mode.
                    //If both are activated, we cannot send
                    if (__BLEInstallKeyboard && !__BLEInstallMouse) {
                        hids_device_send_boot_keyboard_input_report(_con_handle, &(((const uint8_t *)_sendReport)[1]), _sendReportLen);
                    }
                    if (__BLEInstallMouse && !__BLEInstallKeyboard) {
                        hids_device_send_boot_mouse_input_report(_con_handle, &(((const uint8_t *)_sendReport)[1]), _sendReportLen);
                    }
                    if (__BLEInstallMouse && __BLEInstallJoystick) {
                        printf("Error: BLE HID in boot mode, but mouse & keyboard are active\n");
                    }
                    break;
                case 1:
                    reportID = ((const uint8_t *)_sendReport)[0];
                    result = hids_device_send_input_report_for_id(_con_handle, (uint16_t)reportID, &(((const uint8_t *)_sendReport)[1]), _sendReportLen - 1);
                    if (result) {
                        Serial.printf("Error sending %d - report ID: %d\n", result, reportID);
                    }
                    break;
                default:
                    break;
                }
                _needToSend = false;
                if (_onCanSendNow) {
                    _onCanSendNow(type, channel, packet, size);
                }
                break;
            }
            break;
        default:
            break;
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
        return _con_handle != HCI_CON_HANDLE_INVALID;
    }

    bool send(void *rpt, int len) {
        // Wait for another report to be sent
        while (connected() && _needToSend) {
            /* noop busy wait */
        }
        __lockBluetooth();
        if (connected()) {
            _needToSend = true;
            _sendReport = rpt;
            _sendReportLen = len;
            hids_device_request_can_send_now_event(_con_handle);
        }
        __unlockBluetooth();
        while (connected() && _needToSend) {
            /* noop busy wait */
        }
        return connected();
    }

    bool setBattery(int level) {
        if (!_running || (level < 0) || (level > 100)) {
            return false;
        }
        battery_service_server_set_battery_value(level);
        return true;
    }

    uint8_t *_attdb = nullptr;
    int _attdbLen = 0;

private:
    bool _running = false;

    BTCallback _onOpened = nullptr;
    BTCallback _onClosed = nullptr;
    BTCallback _onCanSendNow = nullptr;

    btstack_packet_callback_registration_t _hci_event_callback_registration;
    btstack_packet_callback_registration_t _sm_event_callback_registration;
    uint8_t _battery = 100;
    hci_con_handle_t _con_handle = HCI_CON_HANDLE_INVALID;
    uint8_t _protocol_mode = 1;

    void _buildAdvData(const char *completeLocalName, uint16_t appearance) {
        free(_advData);
        _advDataLen = 13 + strlen(completeLocalName);
        _advData = (uint8_t*) malloc(_advDataLen);
        int i = 0;
        // Flags general discoverable, BR/EDR not supported
        // 0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
        _advData[i++] = 0x02;
        _advData[i++] = BLUETOOTH_DATA_TYPE_FLAGS;
        _advData[i++] = 0x06;
        // Name
        // 0x0d, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME,
        _advData[i++] = 1 + strlen(completeLocalName);
        _advData[i++] = BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME;
        memcpy(_advData + i, completeLocalName, strlen(completeLocalName));
        i += strlen(completeLocalName);
        // 16-bit Service UUIDs
        // 0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
        _advData[i++] = 0x03;
        _advData[i++] = BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS;
        _advData[i++] = ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff;
        _advData[i++] = ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8;
        // Appearance HID - Keyboard (Category 15, Sub-Category 1)
        // 0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC1, 0x03,
        _advData[i++] = 0x03;
        _advData[i++] = BLUETOOTH_DATA_TYPE_APPEARANCE;
        _advData[i++] = appearance & 0xff;
        _advData[i++] = appearance >> 8;
    }
    uint8_t *_advData = nullptr;
    uint8_t _advDataLen = 0;
    hids_device_report_t *_reportStorage = nullptr;

    void _buildAttdb(const char *hidName) {
        free(_attdb);
        //add up all different parts of ATT DB
        _attdbLen = sizeof(_attdb_head) + 8 + strlen(hidName) + sizeof(_attdb_tail) + sizeof(_attdb_batt_hidhead) + sizeof(_attdb_char);
        //reports
        _attdbLen += sizeof(_attdb_kbd_report) + sizeof(_attdb_mouse_report) + sizeof(_attdb_joystick_report);
        //additional boot characteristics
        _attdbLen += sizeof(_attdb_kbd_boot) + sizeof(_attdb_mouse_boot);

        _attdb = (uint8_t *) malloc(_attdbLen);
        memcpy(_attdb, _attdb_head, sizeof(_attdb_head));
        // 0x0003 VALUE CHARACTERISTIC-GAP_DEVICE_NAME - READ -'HID Mouse'
        // READ_ANYBODY
        // 0x11, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x2a, 0x48, 0x49, 0x44, 0x20, 0x4d, 0x6f, 0x75, 0x73, 0x65,
        int i = sizeof(_attdb_head);
        _attdb[i++] = 8 + strlen(hidName);
        _attdb[i++] = 0x00;
        _attdb[i++] = 0x02;
        _attdb[i++] = 0x00;
        _attdb[i++] = 0x03;
        _attdb[i++] = 0x00;
        _attdb[i++] = 0x00;
        _attdb[i++] = 0x2a;
        memcpy(_attdb + i, hidName, strlen(hidName));
        i += strlen(hidName);

        memcpy(_attdb + i, _attdb_batt_hidhead, sizeof(_attdb_batt_hidhead));
        i += sizeof(_attdb_batt_hidhead);

        //1.) KBD report mode
        memcpy(_attdb + i, _attdb_kbd_report, sizeof(_attdb_kbd_report));
        i += sizeof(_attdb_kbd_report);

        //2.) mouse report mode
        memcpy(_attdb + i, _attdb_mouse_report, sizeof(_attdb_mouse_report));
        i += sizeof(_attdb_mouse_report);

        //3.) joystick report mode
        memcpy(_attdb + i, _attdb_joystick_report, sizeof(_attdb_joystick_report));
        i += sizeof(_attdb_joystick_report);
        //4.) report characteristics
        memcpy(_attdb + i, _attdb_char, sizeof(_attdb_char));
        i += sizeof(_attdb_char);

        //5.) KBD boot mode (always included)
        memcpy(_attdb + i, _attdb_kbd_boot, sizeof(_attdb_kbd_boot));
        i += sizeof(_attdb_kbd_boot);

        //6.) mouse boot mode (always included)
        memcpy(_attdb + i, _attdb_mouse_boot, sizeof(_attdb_mouse_boot));
        i += sizeof(_attdb_mouse_boot);
        //7.) tail (report)
        memcpy(_attdb + i, _attdb_tail, sizeof(_attdb_tail));
    }

    static constexpr const uint8_t _attdb_head[] = {
        // ATT DB Version
        1,

        // 0x0001 PRIMARY_SERVICE-GAP_SERVICE
        0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,
        // 0x0002 CHARACTERISTIC-GAP_DEVICE_NAME - READ
        0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x02, 0x03, 0x00, 0x00, 0x2a,
    };

    static constexpr const uint8_t _attdb_batt_hidhead[] = {
        // #import <battery_service.gatt> -- BEGIN
        // Specification Type org.bluetooth.service.battery_service
        // https://www.bluetooth.com/api/gatt/xmlfile?xmlFileName=org.bluetooth.service.battery_service.xml
        // Battery Service 180F
        // 0x0004 PRIMARY_SERVICE-ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE
        0x0a, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x28, 0x0f, 0x18,
        // 0x0005 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL - DYNAMIC | READ | NOTIFY
        0x0d, 0x00, 0x02, 0x00, 0x05, 0x00, 0x03, 0x28, 0x12, 0x06, 0x00, 0x19, 0x2a,
        // 0x0006 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL - DYNAMIC | READ | NOTIFY
        // READ_ANYBODY
        0x08, 0x00, 0x02, 0x01, 0x06, 0x00, 0x19, 0x2a,
        // 0x0007 CLIENT_CHARACTERISTIC_CONFIGURATION
        // READ_ANYBODY, WRITE_ANYBODY
        0x0a, 0x00, 0x0e, 0x01, 0x07, 0x00, 0x02, 0x29, 0x00, 0x00,
        // #import <battery_service.gatt> -- END
        // add Device ID Service


        // #import <device_information_service.gatt> -- BEGIN
        // Specification Type org.bluetooth.service.device_information
        // https://www.bluetooth.com/api/gatt/xmlfile?xmlFileName=org.bluetooth.service.device_information.xml
        // Device Information 180A
        // 0x0008 PRIMARY_SERVICE-ORG_BLUETOOTH_SERVICE_DEVICE_INFORMATION
        0x0a, 0x00, 0x02, 0x00, 0x08, 0x00, 0x00, 0x28, 0x0a, 0x18,
        // 0x0009 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_MANUFACTURER_NAME_STRING - DYNAMIC | READ
        0x0d, 0x00, 0x02, 0x00, 0x09, 0x00, 0x03, 0x28, 0x02, 0x0a, 0x00, 0x29, 0x2a,
        // 0x000a VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_MANUFACTURER_NAME_STRING - DYNAMIC | READ
        // READ_ANYBODY
        0x08, 0x00, 0x02, 0x01, 0x0a, 0x00, 0x29, 0x2a,
        // 0x000b CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_MODEL_NUMBER_STRING - DYNAMIC | READ
        0x0d, 0x00, 0x02, 0x00, 0x0b, 0x00, 0x03, 0x28, 0x02, 0x0c, 0x00, 0x24, 0x2a,
        // 0x000c VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_MODEL_NUMBER_STRING - DYNAMIC | READ
        // READ_ANYBODY
        0x08, 0x00, 0x02, 0x01, 0x0c, 0x00, 0x24, 0x2a,
        // 0x000d CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_SERIAL_NUMBER_STRING - DYNAMIC | READ
        0x0d, 0x00, 0x02, 0x00, 0x0d, 0x00, 0x03, 0x28, 0x02, 0x0e, 0x00, 0x25, 0x2a,
        // 0x000e VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_SERIAL_NUMBER_STRING - DYNAMIC | READ
        // READ_ANYBODY
        0x08, 0x00, 0x02, 0x01, 0x0e, 0x00, 0x25, 0x2a,
        // 0x000f CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_HARDWARE_REVISION_STRING - DYNAMIC | READ
        0x0d, 0x00, 0x02, 0x00, 0x0f, 0x00, 0x03, 0x28, 0x02, 0x10, 0x00, 0x27, 0x2a,
        // 0x0010 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_HARDWARE_REVISION_STRING - DYNAMIC | READ
        // READ_ANYBODY
        0x08, 0x00, 0x02, 0x01, 0x10, 0x00, 0x27, 0x2a,
        // 0x0011 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_FIRMWARE_REVISION_STRING - DYNAMIC | READ
        0x0d, 0x00, 0x02, 0x00, 0x11, 0x00, 0x03, 0x28, 0x02, 0x12, 0x00, 0x26, 0x2a,
        // 0x0012 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_FIRMWARE_REVISION_STRING - DYNAMIC | READ
        // READ_ANYBODY
        0x08, 0x00, 0x02, 0x01, 0x12, 0x00, 0x26, 0x2a,
        // 0x0013 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_SOFTWARE_REVISION_STRING - DYNAMIC | READ
        0x0d, 0x00, 0x02, 0x00, 0x13, 0x00, 0x03, 0x28, 0x02, 0x14, 0x00, 0x28, 0x2a,
        // 0x0014 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_SOFTWARE_REVISION_STRING - DYNAMIC | READ
        // READ_ANYBODY
        0x08, 0x00, 0x02, 0x01, 0x14, 0x00, 0x28, 0x2a,
        // 0x0015 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_SYSTEM_ID - DYNAMIC | READ
        0x0d, 0x00, 0x02, 0x00, 0x15, 0x00, 0x03, 0x28, 0x02, 0x16, 0x00, 0x23, 0x2a,
        // 0x0016 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_SYSTEM_ID - DYNAMIC | READ
        // READ_ANYBODY
        0x08, 0x00, 0x02, 0x01, 0x16, 0x00, 0x23, 0x2a,
        // 0x0017 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_IEEE_11073_20601_REGULATORY_CERTIFICATION_DATA_LIST - DYNAMIC | READ
        0x0d, 0x00, 0x02, 0x00, 0x17, 0x00, 0x03, 0x28, 0x02, 0x18, 0x00, 0x2a, 0x2a,
        // 0x0018 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_IEEE_11073_20601_REGULATORY_CERTIFICATION_DATA_LIST - DYNAMIC | READ
        // READ_ANYBODY
        0x08, 0x00, 0x02, 0x01, 0x18, 0x00, 0x2a, 0x2a,
        // 0x0019 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_PNP_ID - DYNAMIC | READ
        0x0d, 0x00, 0x02, 0x00, 0x19, 0x00, 0x03, 0x28, 0x02, 0x1a, 0x00, 0x50, 0x2a,
        // 0x001a VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_PNP_ID - DYNAMIC | READ
        // READ_ANYBODY
        0x08, 0x00, 0x02, 0x01, 0x1a, 0x00, 0x50, 0x2a,
        // #import <device_information_service.gatt> -- END

        // Specification Type org.bluetooth.service.human_interface_device
        // https://www.bluetooth.com/api/gatt/xmlfile?xmlFileName=org.bluetooth.service.human_interface_device.xml
        // Human Interface Device 1812
        // 0x001b PRIMARY_SERVICE-ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE
        0x0a, 0x00, 0x02, 0x00, 0x1b, 0x00, 0x00, 0x28, 0x12, 0x18,
        // 0x001c CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_PROTOCOL_MODE - DYNAMIC | READ | WRITE_WITHOUT_RESPONSE
        0x0d, 0x00, 0x02, 0x00, 0x1c, 0x00, 0x03, 0x28, 0x06, 0x1d, 0x00, 0x4e, 0x2a,
        // 0x001d VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_PROTOCOL_MODE - DYNAMIC | READ | WRITE_WITHOUT_RESPONSE
        // READ_ANYBODY, WRITE_ANYBODY
        0x08, 0x00, 0x06, 0x01, 0x1d, 0x00, 0x4e, 0x2a,
    };

    static constexpr const uint8_t _attdb_kbd_report[] =  {
        // 0x001e CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT - DYNAMIC | READ | WRITE | NOTIFY | ENCRYPTION_KEY_SIZE_16
        0x0d, 0x00, 0x02, 0x00, 0x1e, 0x00, 0x03, 0x28, 0x1a, 0x1f, 0x00, 0x4d, 0x2a,
        // 0x001f VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT - DYNAMIC | READ | WRITE | NOTIFY | ENCRYPTION_KEY_SIZE_16
        // READ_ENCRYPTED, WRITE_ENCRYPTED, ENCRYPTION_KEY_SIZE=16
        0x08, 0x00, 0x0b, 0xf5, 0x1f, 0x00, 0x4d, 0x2a,
        // 0x0020 CLIENT_CHARACTERISTIC_CONFIGURATION
        // READ_ANYBODY, WRITE_ENCRYPTED, ENCRYPTION_KEY_SIZE=16
        0x0a, 0x00, 0x0f, 0xf1, 0x20, 0x00, 0x02, 0x29, 0x00, 0x00,
        // fixed report id = 1, type = Input (1); keycodes
        // 0x0021 REPORT_REFERENCE-READ-1-1
        0x0a, 0x00, 0x02, 0x00, 0x21, 0x00, 0x08, 0x29, 0x1, 0x1,

        // 0x0022 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT - DYNAMIC | READ | WRITE | NOTIFY | ENCRYPTION_KEY_SIZE_16
        0x0d, 0x00, 0x02, 0x00, 0x22, 0x00, 0x03, 0x28, 0x1a, 0x23, 0x00, 0x4d, 0x2a,
        // 0x0023 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT - DYNAMIC | READ | WRITE | NOTIFY | ENCRYPTION_KEY_SIZE_16
        // READ_ENCRYPTED, WRITE_ENCRYPTED, ENCRYPTION_KEY_SIZE=16
        0x08, 0x00, 0x0b, 0xf5, 0x23, 0x00, 0x4d, 0x2a,
        // 0x0024 CLIENT_CHARACTERISTIC_CONFIGURATION
        // READ_ANYBODY, WRITE_ENCRYPTED, ENCRYPTION_KEY_SIZE=16
        0x0a, 0x00, 0x0f, 0xf1, 0x24, 0x00, 0x02, 0x29, 0x00, 0x00,
        // fixed report id = 2, type = Input (1) consumer
        // 0x0025 REPORT_REFERENCE-READ-2-1
        0x0a, 0x00, 0x02, 0x00, 0x25, 0x00, 0x08, 0x29, 0x2, 0x1,
    };

    static constexpr const uint8_t _attdb_mouse_report[] =  {
        // 0x0026 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT - DYNAMIC | READ | WRITE | NOTIFY | ENCRYPTION_KEY_SIZE_16
        0x0d, 0x00, 0x02, 0x00, 0x26, 0x00, 0x03, 0x28, 0x1a, 0x27, 0x00, 0x4d, 0x2a,
        // 0x0027 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT - DYNAMIC | READ | WRITE | NOTIFY | ENCRYPTION_KEY_SIZE_16
        // READ_ENCRYPTED, WRITE_ENCRYPTED, ENCRYPTION_KEY_SIZE=16
        0x08, 0x00, 0x0b, 0xf5, 0x27, 0x00, 0x4d, 0x2a,
        // 0x0028 CLIENT_CHARACTERISTIC_CONFIGURATION
        // READ_ANYBODY, WRITE_ENCRYPTED, ENCRYPTION_KEY_SIZE=16
        0x0a, 0x00, 0x0f, 0xf1, 0x28, 0x00, 0x02, 0x29, 0x00, 0x00,
        // fixed report id = 3, type = Input (1) mouse
        // 0x0029 REPORT_REFERENCE-READ-3-1
        0x0a, 0x00, 0x02, 0x00, 0x29, 0x00, 0x08, 0x29, 0x3, 0x1,
    };
    static constexpr const uint8_t _attdb_joystick_report[] =  {

        // 0x002a CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT - DYNAMIC | READ | WRITE | NOTIFY | ENCRYPTION_KEY_SIZE_16
        0x0d, 0x00, 0x02, 0x00, 0x2a, 0x00, 0x03, 0x28, 0x1a, 0x2b, 0x00, 0x4d, 0x2a,
        // 0x002b VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT - DYNAMIC | READ | WRITE | NOTIFY | ENCRYPTION_KEY_SIZE_16
        // READ_ENCRYPTED, WRITE_ENCRYPTED, ENCRYPTION_KEY_SIZE=16
        0x08, 0x00, 0x0b, 0xf5, 0x2b, 0x00, 0x4d, 0x2a,
        // 0x002c CLIENT_CHARACTERISTIC_CONFIGURATION
        // READ_ANYBODY, WRITE_ENCRYPTED, ENCRYPTION_KEY_SIZE=16
        0x0a, 0x00, 0x0f, 0xf1, 0x2c, 0x00, 0x02, 0x29, 0x00, 0x00,
        // fixed report id = 4, type = Input (1) gamepad
        // 0x002d REPORT_REFERENCE-READ-4-1
        0x0a, 0x00, 0x02, 0x00, 0x2d, 0x00, 0x08, 0x29, 0x4, 0x1,
    };

    static constexpr const uint8_t _attdb_char[] =  {
        // 0x002e CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT - DYNAMIC | READ | WRITE | ENCRYPTION_KEY_SIZE_16
        0x0d, 0x00, 0x02, 0x00, 0x2e, 0x00, 0x03, 0x28, 0x0a, 0x2f, 0x00, 0x4d, 0x2a,
        // 0x002f VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT - DYNAMIC | READ | WRITE | ENCRYPTION_KEY_SIZE_16
        // READ_ENCRYPTED, WRITE_ENCRYPTED, ENCRYPTION_KEY_SIZE=16
        0x08, 0x00, 0x0b, 0xf5, 0x2f, 0x00, 0x4d, 0x2a,
        // fixed report id = 5, type = Feature (3)
        // 0x0030 REPORT_REFERENCE-READ-5-3
        0x0a, 0x00, 0x02, 0x00, 0x30, 0x00, 0x08, 0x29, 0x5, 0x3,
        // 0x0031 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT_MAP - DYNAMIC | READ
        0x0d, 0x00, 0x02, 0x00, 0x31, 0x00, 0x03, 0x28, 0x02, 0x32, 0x00, 0x4b, 0x2a,
        // 0x0032 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT_MAP - DYNAMIC | READ
        // READ_ANYBODY
        0x08, 0x00, 0x02, 0x01, 0x32, 0x00, 0x4b, 0x2a,
    };

    static constexpr const uint8_t _attdb_kbd_boot[] =  {
        // 0x0033 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT - DYNAMIC | READ | WRITE | NOTIFY
        0x0d, 0x00, 0x02, 0x00, 0x33, 0x00, 0x03, 0x28, 0x1a, 0x34, 0x00, 0x22, 0x2a,
        // 0x0034 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT - DYNAMIC | READ | WRITE | NOTIFY
        // READ_ANYBODY, WRITE_ANYBODY
        0x08, 0x00, 0x0a, 0x01, 0x34, 0x00, 0x22, 0x2a,
        // 0x0035 CLIENT_CHARACTERISTIC_CONFIGURATION
        // READ_ANYBODY, WRITE_ANYBODY
        0x0a, 0x00, 0x0e, 0x01, 0x35, 0x00, 0x02, 0x29, 0x00, 0x00,


        // 0x0036 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_OUTPUT_REPORT - DYNAMIC | READ | WRITE | WRITE_WITHOUT_RESPONSE
        0x0d, 0x00, 0x02, 0x00, 0x36, 0x00, 0x03, 0x28, 0x0e, 0x37, 0x00, 0x32, 0x2a,
        // 0x0037 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_OUTPUT_REPORT - DYNAMIC | READ | WRITE | WRITE_WITHOUT_RESPONSE
        // READ_ANYBODY, WRITE_ANYBODY
        0x08, 0x00, 0x0e, 0x01, 0x37, 0x00, 0x32, 0x2a,
    };

    static constexpr const uint8_t _attdb_mouse_boot[] =  {
        // 0x0038 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_MOUSE_INPUT_REPORT - DYNAMIC | READ | WRITE | NOTIFY
        0x0d, 0x00, 0x02, 0x00, 0x38, 0x00, 0x03, 0x28, 0x1a, 0x39, 0x00, 0x33, 0x2a,
        // 0x0039 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_MOUSE_INPUT_REPORT - DYNAMIC | READ | WRITE | NOTIFY
        // READ_ANYBODY, WRITE_ANYBODY
        0x08, 0x00, 0x0a, 0x01, 0x39, 0x00, 0x33, 0x2a,
        // 0x003a CLIENT_CHARACTERISTIC_CONFIGURATION
        // READ_ANYBODY, WRITE_ANYBODY
        0x0a, 0x00, 0x0e, 0x01, 0x3a, 0x00, 0x02, 0x29, 0x00, 0x00,
    };

    static constexpr const uint8_t _attdb_tail[] =  {
        // bcdHID = 0x101 (v1.0.1), bCountryCode 0, remote wakeable = 0 | normally connectable 2
        // 0x003b CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_HID_INFORMATION - READ
        0x0d, 0x00, 0x02, 0x00, 0x3b, 0x00, 0x03, 0x28, 0x02, 0x3c, 0x00, 0x4a, 0x2a,
        // 0x003c VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_HID_INFORMATION - READ -'01 01 00 02'
        // READ_ANYBODY
        0x0c, 0x00, 0x02, 0x00, 0x3c, 0x00, 0x4a, 0x2a, 0x01, 0x01, 0x00, 0x02,
        // 0x003d CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_HID_CONTROL_POINT - DYNAMIC | WRITE_WITHOUT_RESPONSE
        0x0d, 0x00, 0x02, 0x00, 0x3d, 0x00, 0x03, 0x28, 0x04, 0x3e, 0x00, 0x4c, 0x2a,
        // 0x003e VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_HID_CONTROL_POINT - DYNAMIC | WRITE_WITHOUT_RESPONSE
        // WRITE_ANYBODY
        0x08, 0x00, 0x04, 0x01, 0x3e, 0x00, 0x4c, 0x2a,
        // END
        0x00, 0x00,
    };

    volatile bool _needToSend = false;
    void *_sendReport;
    uint8_t _sendReportID;
    int _sendReportLen;
};
