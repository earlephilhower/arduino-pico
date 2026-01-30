/*
    ATT Dispatch wrappers for Bluetooth locking

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

#if defined(ENABLE_CLASSIC) || defined(ENABLE_BLE)

#include <stdint.h>
#include <stdbool.h>

// Forward declarations for wrapped functions
extern void __real_att_dispatch_register_client(void *packet_handler);
extern void __real_att_dispatch_register_server(void *packet_handler);
extern void __real_att_dispatch_classic_register_service(void);
extern uint8_t __real_att_dispatch_classic_connect(void *address, uint16_t l2cap_psm, uint16_t *out_cid);
extern bool __real_att_dispatch_client_can_send_now(uint16_t con_handle);
extern bool __real_att_dispatch_server_can_send_now(uint16_t con_handle);
extern void __real_att_dispatch_client_request_can_send_now_event(uint16_t con_handle);
extern void __real_att_dispatch_server_request_can_send_now_event(uint16_t con_handle);
extern void __real_att_dispatch_server_mtu_exchanged(uint16_t con_handle, uint16_t new_mtu);
extern void __real_att_dispatch_client_mtu_exchanged(uint16_t con_handle, uint16_t new_mtu);

// BluetoothLock is a C++ class, but we need to call it from C
// So we use extern C functions that will be defined in C++
extern void __lockBluetooth(void);
extern void __unlockBluetooth(void);

void __wrap_att_dispatch_register_client(void *packet_handler) {
    __lockBluetooth();
    __real_att_dispatch_register_client(packet_handler);
    __unlockBluetooth();
}

void __wrap_att_dispatch_register_server(void *packet_handler) {
    __lockBluetooth();
    __real_att_dispatch_register_server(packet_handler);
    __unlockBluetooth();
}

void __wrap_att_dispatch_classic_register_service(void) {
    __lockBluetooth();
    __real_att_dispatch_classic_register_service();
    __unlockBluetooth();
}

uint8_t __wrap_att_dispatch_classic_connect(void *address, uint16_t l2cap_psm, uint16_t *out_cid) {
    __lockBluetooth();
    uint8_t result = __real_att_dispatch_classic_connect(address, l2cap_psm, out_cid);
    __unlockBluetooth();
    return result;
}

bool __wrap_att_dispatch_client_can_send_now(uint16_t con_handle) {
    __lockBluetooth();
    bool result = __real_att_dispatch_client_can_send_now(con_handle);
    __unlockBluetooth();
    return result;
}

bool __wrap_att_dispatch_server_can_send_now(uint16_t con_handle) {
    __lockBluetooth();
    bool result = __real_att_dispatch_server_can_send_now(con_handle);
    __unlockBluetooth();
    return result;
}

void __wrap_att_dispatch_client_request_can_send_now_event(uint16_t con_handle) {
    __lockBluetooth();
    __real_att_dispatch_client_request_can_send_now_event(con_handle);
    __unlockBluetooth();
}

void __wrap_att_dispatch_server_request_can_send_now_event(uint16_t con_handle) {
    __lockBluetooth();
    __real_att_dispatch_server_request_can_send_now_event(con_handle);
    __unlockBluetooth();
}

void __wrap_att_dispatch_server_mtu_exchanged(uint16_t con_handle, uint16_t new_mtu) {
    __lockBluetooth();
    __real_att_dispatch_server_mtu_exchanged(con_handle, new_mtu);
    __unlockBluetooth();
}

void __wrap_att_dispatch_client_mtu_exchanged(uint16_t con_handle, uint16_t new_mtu) {
    __lockBluetooth();
    __real_att_dispatch_client_mtu_exchanged(con_handle, new_mtu);
    __unlockBluetooth();
}

#endif
