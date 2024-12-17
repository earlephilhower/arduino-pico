/*
    PicoBluetoothHID.h - Simple wrapper for BT-HID objects like
    keyboards, mice, gamepads.
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

#include "PicoBluetoothHID.h"

#define CCALLBACKNAME _BTHIDCB
#include <ctocppcallback.h>

#define PACKETHANDLERCB(class, cbFcn) \
  (CCALLBACKNAME<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__>::func = std::bind(&class::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), \
   static_cast<btstack_packet_handler_t>(CCALLBACKNAME<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__ - 1>::callback))

bool PicoBluetoothHID_::startHID(const char *localName, const char *hidName, uint16_t hidClass, uint8_t hidSubclass, const uint8_t *hidDescriptor, uint16_t hidDescriptorSize) {
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
    _hci_event_callback_registration.callback = PACKETHANDLERCB(PicoBluetoothHID_, packetHandler);
    hci_add_event_handler(&_hci_event_callback_registration);

    // register for HID events
    hid_device_register_packet_handler(PACKETHANDLERCB(PicoBluetoothHID_, packetHandler));

    hci_power_control(HCI_POWER_ON);
    return true;
}

PicoBluetoothHID_ PicoBluetoothHID;
