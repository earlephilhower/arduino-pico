/*
    BLEServer - BLE top-level server (holds services)
    Copyright (c) 2026 Earle F. Philhower, III.  All rights reserved.

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
#include <btstack.h>
#include "BLE.h"
#include "BLEServer.h"

#include <ble/att_db_util.h>
#include <pico/async_context.h>
#include <pico/cyw43_arch.h>
#include <list>
#include <BluetoothLock.h>

BLEServer::BLEServer() {
    _name = strdup("");
}

BLEServer::~BLEServer() {
    free(_name);
}

void BLEServer::addService(BLEService *s) {
    _svc.push_back(s);
}

void BLEServer::prepareAdvertising(BLEAdvertising *adv) {
    att_db_util_init();
    att_db_util_hash_init();

    // Add the GAP name service and constant characteristic
    att_db_util_add_service_uuid16(GAP_SERVICE_UUID);
    _nameHandle = att_db_util_add_characteristic_uuid16(GAP_DEVICE_NAME_UUID, ATT_PROPERTY_READ | ATT_PROPERTY_DYNAMIC, ATT_SECURITY_NONE, ATT_SECURITY_NONE, nullptr, 0);  //(uint8_t *)_name, strlen(_name));

    BLEUUID uuid;
    for (auto s : _svc) {
        s->addATTDB();
        Serial.println(s->_svc.toString());
        //        if (s->_svc != BLEUUID(ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE)) {
        uuid = s->_svc;
        //        }
    }
    if (uuid) {
        adv->setPartialUUID(uuid);
    }

    // TODO - add HASH characteristic.  Needs HCI already running and needs to be include
    // att_db_util_add_service_uuid16(0x1801);
    // att_db_util_add_characteristic_uuid16(0x2b2a, ATT_PROPERTY_READ, ATT_SECURITY_NONE, ATT_SECURITY_NONE, zeros, 16);
    // att_db_util_hash_calc(&cmac_context, cmac_calculated, &gatt_hash_calculated, NULL);
    // <patch in cmac_calculated into the just added characteristic>
}

void BLEServer::setName(const char *name) {
    free(_name);
    _name = strdup(name);
}

String BLEServer::toString() {
    prepareAdvertising(BLE.advertising());
    uint8_t *p = att_db_util_get_address();
    int len = att_db_util_get_size();
    String o = "GATTDB: ";
    for (int i = 0; i < len; i ++) {
        char b[3];
        sprintf(b, "%02x", p[i]);
        o += String(b);
    }
    return o;
}

bool BLEServer::setCallbacks(BLEServerCallbacks *cb) {
    _serverCB = cb;
    return true;
}

uint16_t BLEServer::readHandler(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    // Will be running in BT ctx already
    (void)con_handle;
    if (attribute_handle == _nameHandle) {
        return att_read_callback_handle_blob((uint8_t *)_name, strlen(_name), offset, buffer, buffer_size);
    }
    for (auto s : _svc) {
        uint16_t len = s->handleRead(attribute_handle, offset, buffer, buffer_size);
        if (len) {
            return len;
        }
    }
    return 0;
}

int BLEServer::writeHandler(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    // Will be running in BT ctx already
    for (auto s : _svc) {
        int ret = s->handleWrite(attribute_handle, transaction_mode, offset, buffer, buffer_size);
        if (ret) {
            return ret;
        }
    }
    return 0;
}

void BLEServer::packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size) {
    // Will be running in BT ctx already
    if (type != HCI_EVENT_PACKET) {
        return;
    }

    uint8_t event_type = hci_event_packet_get_type(packet);

    switch (event_type) {

    case HCI_EVENT_META_GAP:
        switch (hci_event_gap_meta_get_subevent_code(packet)) {
        case GAP_SUBEVENT_LE_CONNECTION_COMPLETE:
            con_handle = gap_subevent_le_connection_complete_get_connection_handle(packet);
            DEBUGV("Connection complete %04x\n", con_handle);
            for (auto s : _svc) {
                s->setConHandle(con_handle);
            }
            if (_serverCB) {
                _serverCB->onConnect(this);
            }
            //            sm_request_pairing(con_handle);
            break;
        }
        break;
#if 0
    case SM_EVENT_JUST_WORKS_REQUEST:
        printf("Just Works requested\n");
        sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
        break;
    case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
        printf("Confirming numeric comparison: %" PRIu32 "\n", sm_event_numeric_comparison_request_get_passkey(packet));
        sm_numeric_comparison_confirm(sm_event_passkey_display_number_get_handle(packet));
        break;
    case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
        printf("Display Passkey: %" PRIu32 "\n", sm_event_passkey_display_number_get_passkey(packet));
        break;
    case SM_EVENT_IDENTITY_CREATED:
        sm_event_identity_created_get_identity_address(packet, addr);
        printf("Identity created: type %u address %s\n", sm_event_identity_created_get_identity_addr_type(packet), bd_addr_to_str(addr));
        break;
    case SM_EVENT_IDENTITY_RESOLVING_SUCCEEDED:
        sm_event_identity_resolving_succeeded_get_identity_address(packet, addr);
        printf("Identity resolved: type %u address %s\n", sm_event_identity_resolving_succeeded_get_identity_addr_type(packet), bd_addr_to_str(addr));
        break;
    case SM_EVENT_IDENTITY_RESOLVING_FAILED:
        sm_event_identity_created_get_address(packet, addr);
        printf("Identity resolving failed\n");
        break;
    case SM_EVENT_PAIRING_STARTED:
        printf("Pairing started\n");
        break;
    case SM_EVENT_PAIRING_COMPLETE:
        switch (sm_event_pairing_complete_get_status(packet)) {
        case ERROR_CODE_SUCCESS:
            printf("Pairing complete, success\n");
            break;
        case ERROR_CODE_CONNECTION_TIMEOUT:
            printf("Pairing failed, timeout\n");
            break;
        case ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION:
            printf("Pairing failed, disconnected\n");
            break;
        case ERROR_CODE_AUTHENTICATION_FAILURE:
            printf("Pairing failed, authentication failure with reason = %u\n", sm_event_pairing_complete_get_reason(packet));
            break;
        default:
            break;
        }
        break;
#endif
    // Disconnected from a client
    case HCI_EVENT_DISCONNECTION_COMPLETE:
        if (con_handle) {
            for (auto s : _svc) {
                s->disconnected();
            }
            if (_serverCB) {
                _serverCB->onDisconnect(this);
            }
            con_handle = 0;
        }
        break;

    default:
        break;
    }
}
