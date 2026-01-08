/*
    BLE - Top level class for managing BLE servers and clients
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

#include "BLE.h"
#include <btstack.h>
#include <BluetoothLock.h>

#define CCALLBACKNAME _BLECB
#include <ctocppcallback.h>

#define PACKETHANDLERCB(class, cbFcn) \
  (_BLECB<void(uint8_t, uint16_t, uint8_t *, uint16_t), __COUNTER__>::func = std::bind(&class ::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), \
   static_cast<btstack_packet_handler_t>(_BLECB<void(uint8_t, uint16_t, uint8_t *, uint16_t), __COUNTER__ - 1>::callback))

#define SERVICEWRITEHANDLERCB(class, cbFcn) \
  (_BLECB<int(hci_con_handle_t, uint16_t, uint16_t, uint16_t, uint8_t *, uint16_t), __COUNTER__>::func = std::bind(&class ::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6), \
   static_cast<att_write_callback_t>(_BLECB<int(hci_con_handle_t, uint16_t, uint16_t, uint16_t, uint8_t *, uint16_t), __COUNTER__ - 1>::callback))

#define SERVICEREADHANDLERCB(class, cbFcn) \
  (_BLECB<uint16_t(hci_con_handle_t, uint16_t, uint16_t, uint8_t *, uint16_t), __COUNTER__>::func = std::bind(&class ::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5), \
   static_cast<att_read_callback_t>(_BLECB<uint16_t(hci_con_handle_t, uint16_t, uint16_t, uint8_t *, uint16_t), __COUNTER__ - 1>::callback))




BLEClass BLE;

void BLEClass::begin(String name) {
    /*btstack_packet_callback_registration_t hci_event_callback_registration;*/
    hci_event_callback_registration = realloc(hci_event_callback_registration, sizeof(btstack_packet_callback_registration_t));
    /*att_service_handler_t service_handler;*/
    service_handler = realloc(service_handler, sizeof(att_service_handler_t));

    _advertising.setName(name.c_str());

    l2cap_init();
    sm_init();


    btstack_packet_callback_registration_t *h = (btstack_packet_callback_registration_t *)hci_event_callback_registration;
    bzero(h, sizeof(*h));
    h->callback = PACKETHANDLERCB(BLEClass, packetHandler);
    hci_add_event_handler(h);
    sm_add_event_handler(h);

    hci_power_control(HCI_POWER_ON);

    // Wait until we have an addr...
    while (!_addr) {
        delay(10);
    }
}

BLEAddress BLEClass::address() {
    return _addr;
}

BLEAdvertising *BLEClass::advertising() {
    return &_advertising;
}

void BLEClass::startAdvertising() {
    if (_server) {
        _server->prepareAdvertising(&_advertising);
    } else {
        // Build ATT with only the name
        att_db_util_init();
        att_db_util_add_service_uuid16(GAP_SERVICE_UUID);
        att_db_util_add_characteristic_uuid16(GAP_DEVICE_NAME_UUID, ATT_PROPERTY_READ, ATT_SECURITY_NONE, ATT_SECURITY_NONE, (uint8_t *)_advertising.getName(), strlen(_advertising.getName()));
    }

    att_server_init(att_db_util_get_address(), nullptr, nullptr);
    att_server_register_packet_handler(PACKETHANDLERCB(BLEClass, packetHandler));

    att_service_handler_t *s = (att_service_handler_t *)service_handler;
    bzero(s, sizeof(*s));
    s->start_handle = 0;
    s->end_handle = 0xFFFF;  // We could register this at service level
    s->read_callback = SERVICEREADHANDLERCB(BLEClass, readHandler);
    s->write_callback = SERVICEWRITEHANDLERCB(BLEClass, writeHandler);
    att_server_register_service_handler(s);

    gap_advertisements_set_params(_advertising.getAdvertisingIntervalMin(), _advertising.getAdvertisingIntervalMax(), 0, 0, _addr.rawAddress(), 0x07, 0x00);
    _advertising.build();
    gap_advertisements_set_data(_advertising.getBlobLen(), (uint8_t *)_advertising.getBlob());
    gap_advertisements_enable(1);
}

void BLEClass::stopAdvertising() {
    gap_advertisements_enable(0);
    att_server_deinit();
}

BLEServer *BLEClass::server() {
    if (!_server) {
        _server = new BLEServer;
        _server->setName(_advertising.getName());
    }
    return _server;
}

void BLEClass::clearScan() {
    _scanResults.clear();
    _scanResults.shrink_to_fit();
}


BLEScanReport *BLEClass::scan(int timeoutSec, bool active, int intervalms, int windowms) {
    clearScan();

    if (!_addr || !timeoutSec) {
        return &_scanResults;  // BT hasn't started up
    }

    // Because we really don't want to be allocating space inside the BTStack
    // IRQ-level callback, we'll use the main app to resize things as they
    // go along.  This requires the main app while() loop to run often enough
    // so the pre-allocated spaces never run out.  Please forgive me.
    _scanResults.reserve(20);

    // Start the scan process
    gap_set_scan_params(active ? 1 : 0, intervalms, windowms, 0 /*all*/);
    gap_start_scan();

    uint32_t startTime = millis();
    uint32_t timeout = timeoutSec * 1000;
    while ((millis() - startTime) < timeout) {
        do { // Ensure we only lock for the first bit of the wait pass
            BluetoothLock l;
            if (_scanResults.capacity() < (_scanResults.size() + 10)) {
                // Add 10 more spaces in user context.  BT can't run, we own the lock
                _scanResults.reserve(_scanResults.size() + 20);
            }
        } while (0);
        delay(10); // Let some work get done by the BT IRQ
    }
    gap_stop_scan();
    _scanResults.shrink_to_fit();
    return &_scanResults;
}

uint16_t BLEClass::readHandler(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    if (_server) {
        return _server->readHandler(con_handle, attribute_handle, offset, buffer, buffer_size);
    } else {
        return 0;
    }
}

int BLEClass::writeHandler(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    if (_server) {
        return _server->writeHandler(con_handle, attribute_handle, transaction_mode, offset, buffer, buffer_size);
    } else {
        return 0;
    }
}

void BLEClass::packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size) {
    // Will be running in BT ctx already, no need to do any locking
    if (type != HCI_EVENT_PACKET) {
        return;
    }

    uint8_t event_type = hci_event_packet_get_type(packet);

    switch (event_type) {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
            // Nothing to do until it's a "we're alive" notify
            return;
        }
        bd_addr_t addr;
        gap_local_bd_addr(addr);
        _addr = BLEAddress(addr);
        break;
    case GAP_EVENT_ADVERTISING_REPORT:
        advertisementHandler(packet);
        break;
    default:
        break;
    }

    if (_server) {
        _server->packetHandler(type, channel, packet, size);
    }
}

void BLEClass::advertisementHandler(uint8_t *packet) {
    BLEAdvertising adv;

    bd_addr_t address;
    gap_event_advertising_report_get_address(packet, address);
    int address_type = gap_event_advertising_report_get_address_type(packet);
    adv.setAddress(BLEAddress((uint8_t *)address, address_type == BD_ADDR_TYPE_LE_PUBLIC ? BLEPublicAddress : BLERandomAddress));

    // TODO - Can advertisements change?  This should allow us to make out discovery the superset of all advertised values
    BLEAddress ba = BLEAddress(address);
    for (size_t i = 0; i < _scanResults.size(); i++) {
        if (_scanResults[i].address() == ba) {
            adv = _scanResults[i];
            break;
        }
    }

    int8_t rssi = gap_event_advertising_report_get_rssi(packet);
    adv.setRSSI(rssi);

    uint8_t adv_size = gap_event_advertising_report_get_data_length(packet);
    const uint8_t *adv_data = gap_event_advertising_report_get_data(packet);
    ad_context_t context; /*passive*/
    for (ad_iterator_init(&context, adv_size, (uint8_t *)adv_data); ad_iterator_has_more(&context); ad_iterator_next(&context)) {
        uint8_t data_type = ad_iterator_get_data_type(&context);
        uint8_t size = ad_iterator_get_data_len(&context);
        const uint8_t *data = ad_iterator_get_data(&context);
        int i;
        // Assigned Numbers GAP
        switch (data_type) {
        case BLUETOOTH_DATA_TYPE_FLAGS:
            // noop
            break;

        case BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS:
            for (i = 0; i < size; i += 2) {
                uint16_t thisuuid = little_endian_read_16(data, i);
                adv.setPartialUUID(BLEUUID(thisuuid));
            }
            break;

        case BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS:
            for (i = 0; i < size; i += 2) {
                uint16_t thisuuid = little_endian_read_16(data, i);
                adv.setOnlyUUID(BLEUUID(thisuuid));
            }
            break;

        case BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS:
            for (i = 0; i < size; i += 16) {
                BLEUUID uuid((uint8_t *)data + i);
                adv.setPartialUUID(uuid);
            }
            break;

        case BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS:
            for (i = 0; i < size; i += 16) {
                BLEUUID uuid((uint8_t *)data + i);
                adv.setOnlyUUID(uuid);
            }
            break;

        case BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME:
        case BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME:
            char buff[32];
            bzero(buff, 32);
            memcpy(buff, data, size);
            adv.setName(buff);
            break;

        case BLUETOOTH_DATA_TYPE_MANUFACTURER_SPECIFIC_DATA:
            adv.setManufacturerData(data, size);
            break;

        case BLUETOOTH_DATA_TYPE_APPEARANCE:
            adv.setAppearance((uint16_t)little_endian_read_16(data, 0));
            break;

        default:
            // DEBUGV("Advertising Data Type 0x%02x not handled yet\n", data_type);
            break;
        }
    }

    // Now see if we should append or overwrite an element.  Use dumb C way to avoid
    // allocations and enable updates
    bool updated = false;
    for (size_t i = 0; i < _scanResults.size(); i++) {
        if (_scanResults[i].address() == ba) {
            _scanResults[i] = adv;
            updated = true;
            break;
        }
    }
    if (!updated) {
        if (_scanResults.size() < _scanResults.capacity()) {
            _scanResults.push_back(adv);
        } else {
            DEBUGV("OOM for new scan result");
        }
    }
}
