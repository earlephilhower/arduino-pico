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

void BluetoothHCI::setBLEName(const char *name) {
    if (_att) {
        free(_att);
        _att = nullptr;
    }
    _att = (uint8_t *)malloc(1 + 0x0a + 0x0d + 0x08 + strlen(name) + 0x02);
    uint8_t *ptr = _att;
    const uint8_t head[] = {
        // ATT DB Version
        1,
        // 0x0001 PRIMARY_SERVICE-GAP_SERVICE
        0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,
        // 0x0002 CHARACTERISTIC-GAP_DEVICE_NAME - READ
        0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x02, 0x03, 0x00, 0x00, 0x2a
    };
    memcpy(ptr, head, sizeof(head));
    ptr += sizeof(head);
    *ptr++ = 8 + strlen(name); // len of item
    *ptr++ = 0x00;
    *ptr++ = 0x02;
    *ptr++ = 0x00;
    *ptr++ = 0x03;
    *ptr++ = 0x00;
    *ptr++ = 0x00;
    *ptr++ = 0x2a;
    memcpy(ptr, name, strlen(name));
    ptr += strlen(name);
    // End it
    *ptr++ = 0x00;
    *ptr++ = 0x00;

    DEBUGV("ATTDB: ");
    for (size_t i = 0; i < 1 + 0x0a + 0x0d + 0x08 + strlen(name) + 0x02; i++) {
        DEBUGV("%02X ", _att[i]);
    }
    DEBUGV("\n");
}

void BluetoothHCI::install() {
    hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);

    // Register for HCI events.
    hci_event_callback_registration.callback = PACKETHANDLERCB(BluetoothHCI, hci_packet_handler);
    hci_add_event_handler(&hci_event_callback_registration);

    // BLE set our visible name
    if (_att) {
        att_server_init(_att, nullptr, nullptr);
    }
}


void BluetoothHCI::begin() {
    _running = true;
    hci_power_control(HCI_POWER_ON);
}

void BluetoothHCI::uninstall() {
    BluetoothLock b;
    hci_remove_event_handler(&hci_event_callback_registration);
    _running = false;
    free(_att);
    _att = nullptr;
}

bool BluetoothHCI::running() {
    return _hciRunning;
}

std::vector<BTDeviceInfo> BluetoothHCI::scan(uint32_t mask, int scanTimeSec, bool async) {
    _scanMask = mask;
    _btdList.reserve(MAX_DEVICES_TO_DISCOVER);
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

std::vector<BTDeviceInfo> BluetoothHCI::scanBLE(uint32_t uuid, int scanTimeSec) {
    _scanMask = uuid;
    _btdList.reserve(MAX_DEVICES_TO_DISCOVER);
    _btdList.clear();
    if (!_running) {
        return _btdList;
    }
    _scanning = true;
    while (!_hciRunning) {
        DEBUGV("HCI::scanBLE(): Waiting for HCI to come up\n");
        delay(10);
    }
    uint32_t inquiryTime = scanTimeSec * 1000;
    if (!inquiryTime) {
        inquiryTime = 1000;
    }
    DEBUGV("HCI::scan(): BLE advertise inquiry start\n");
    // Only need to lock around the inquiry start command, not the wait
    {
        BluetoothLock b;
        gap_set_scan_params(0, 75, 50, 0);  // TODO - anything better for these params?
        gap_start_scan();
    }
    uint32_t scanStart = millis();

    while (_scanning && ((millis() - scanStart) < inquiryTime)) {
        delay(10);
    }
    DEBUGV("HCI::scanBLE(): inquiry end\n");
    gap_stop_scan();

    return _btdList;
}

void BluetoothHCI::scanFree() {
    _btdList.clear();
    _btdList.shrink_to_fit();
}

void BluetoothHCI::parse_advertisement_data(uint8_t *packet) {
    bd_addr_t address;
    gap_event_advertising_report_get_address(packet, address);
    //int event_type = gap_event_advertising_report_get_advertising_event_type(packet);
    int address_type = gap_event_advertising_report_get_address_type(packet);
    int8_t rssi = gap_event_advertising_report_get_rssi(packet);
    uint8_t adv_size = gap_event_advertising_report_get_data_length(packet);
    const uint8_t * adv_data = gap_event_advertising_report_get_data(packet);
    uint16_t uuid = 0;
    const char *nameptr = nullptr;
    int namelen = 0;

    ad_context_t context;
    uint8_t uuid_128[16];
    for (ad_iterator_init(&context, adv_size, (uint8_t *)adv_data) ; ad_iterator_has_more(&context) ; ad_iterator_next(&context)) {
        uint8_t data_type    = ad_iterator_get_data_type(&context);
        uint8_t size         = ad_iterator_get_data_len(&context);
        const uint8_t * data = ad_iterator_get_data(&context);

        //        if (data_type > 0 && data_type < 0x1B){
        //            printf("    %s: ", ad_types[data_type]);
        //        }
        int i;
        // Assigned Numbers GAP

        switch (data_type) {
        case BLUETOOTH_DATA_TYPE_FLAGS:
            // show only first octet, ignore rest
            //for (i=0; i<8;i++){
            //    if (data[0] & (1<<i)){
            //        printf("%s; ", flags[i]);
            //    }

            //}
            break;
        case BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_LIST_OF_16_BIT_SERVICE_SOLICITATION_UUIDS:
            for (i = 0; i < size; i += 2) {
                uint16_t thisuuid = little_endian_read_16(data, i);
                if (!_scanMask || (_scanMask == thisuuid)) {
                    uuid = thisuuid;
                }
                DEBUGV("uuid %02X ", thisuuid);
            }
            break;
        case BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_32_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_32_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_LIST_OF_32_BIT_SERVICE_SOLICITATION_UUIDS:
            for (i = 0; i < size; i += 4) {
                uint32_t thisuuid = little_endian_read_32(data, i);
                if (!_scanMask || (_scanMask == thisuuid)) {
                    uuid = thisuuid;
                }
                DEBUGV("%04" PRIX32, thisuuid);
            }
            break;
        case BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS:
        case BLUETOOTH_DATA_TYPE_LIST_OF_128_BIT_SERVICE_SOLICITATION_UUIDS:
            // Unhandled yet
            reverse_128(data, uuid_128);
            DEBUGV("%s", uuid128_to_str(uuid_128));
            break;
        case BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME:
            if (!nameptr) {
                nameptr = (const char *)data;
                namelen = size;
            }
            for (i = 0; i < size; i++) {
                DEBUGV("%c", (char)(data[i]));
            }
            break;

        case BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME:
            nameptr = (const char *)data;
            namelen = size;
            for (i = 0; i < size; i++) {
                DEBUGV("%c", (char)(data[i]));
            }
            break;
        case BLUETOOTH_DATA_TYPE_TX_POWER_LEVEL:
            DEBUGV("%d dBm", *(int8_t*)data);
            break;
        case BLUETOOTH_DATA_TYPE_SLAVE_CONNECTION_INTERVAL_RANGE:
            DEBUGV("Connection Interval Min = %u ms, Max = %u ms", little_endian_read_16(data, 0) * 5 / 4, little_endian_read_16(data, 2) * 5 / 4);
            break;
        case BLUETOOTH_DATA_TYPE_SERVICE_DATA:
            //printf_hexdump(data, size);
            break;
        case BLUETOOTH_DATA_TYPE_PUBLIC_TARGET_ADDRESS:
        case BLUETOOTH_DATA_TYPE_RANDOM_TARGET_ADDRESS:
            reverse_bd_addr(data, address);
            DEBUGV("%s", bd_addr_to_str(address));
            break;
        case BLUETOOTH_DATA_TYPE_APPEARANCE:
            // https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml
            DEBUGV("%02X", little_endian_read_16(data, 0));
            break;
        case BLUETOOTH_DATA_TYPE_ADVERTISING_INTERVAL:
            DEBUGV("%u ms", little_endian_read_16(data, 0) * 5 / 8);
            break;
        case BLUETOOTH_DATA_TYPE_3D_INFORMATION_DATA:
            //printf_hexdump(data, size);
            break;
        case BLUETOOTH_DATA_TYPE_MANUFACTURER_SPECIFIC_DATA: // Manufacturer Specific Data
            break;
        case BLUETOOTH_DATA_TYPE_CLASS_OF_DEVICE:
        case BLUETOOTH_DATA_TYPE_SIMPLE_PAIRING_HASH_C:
        case BLUETOOTH_DATA_TYPE_SIMPLE_PAIRING_RANDOMIZER_R:
        case BLUETOOTH_DATA_TYPE_DEVICE_ID:
        case BLUETOOTH_DATA_TYPE_SECURITY_MANAGER_OUT_OF_BAND_FLAGS:
        default:
            DEBUGV("Advertising Data Type 0x%2x not handled yet", data_type);
            break;
        }
        DEBUGV("\n");
    }
    DEBUGV("\n");

    if (uuid) {
        bool seen = false;
        for (auto itr = _btdList.begin(); (itr != _btdList.end()) && !seen; itr++) {
            if (!memcmp(itr->address(), address, sizeof(address))) {
                seen = true;
            }
        }
        if (!seen) {
            BTDeviceInfo btd(uuid, address, address_type, rssi, nameptr ? nameptr : "", nameptr ? namelen : 0);
            if (_btdList.size() < MAX_DEVICES_TO_DISCOVER) {
                _btdList.push_back(btd);
            }
        }
    }
}


static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    gatt_client_service_t service;
    gatt_client_characteristic_t characteristic;
    switch (hci_event_packet_get_type(packet)) {
    case GATT_EVENT_SERVICE_QUERY_RESULT:
        gatt_event_service_query_result_get_service(packet, &service);
        //            dump_service(&service);
        //            services[service_count++] = service;
        break;
    case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
        gatt_event_characteristic_query_result_get_characteristic(packet, &characteristic);
        //            dump_characteristic(&characteristic);
        break;
    case GATT_EVENT_QUERY_COMPLETE:
        // GATT_EVENT_QUERY_COMPLETE of search characteristics
        //            if (service_index < service_count) {
        //                service = services[service_index++];
        //                printf("\nGATT browser - CHARACTERISTIC for SERVICE %s, [0x%04x-0x%04x]\n",
        //                    uuid128_to_str(service.uuid128), service.start_group_handle, service.end_group_handle);
        //                gatt_client_discover_characteristics_for_service(handle_gatt_client_event, connection_handle, &service);
        //                break;
        //            }
        //            service_index = 0;
        break;
    default:
        break;
    }
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
            if (_btdList.size() < MAX_DEVICES_TO_DISCOVER) {
                _btdList.push_back(btd);
            }
        }
        break;
    case GAP_EVENT_INQUIRY_COMPLETE:
        _scanning = false;
        break;

    case GAP_EVENT_ADVERTISING_REPORT:
        if (_scanning) {
            parse_advertisement_data(packet);
        }
        break;

    case HCI_EVENT_DISCONNECTION_COMPLETE:
        _hciConn = HCI_CON_HANDLE_INVALID;
        DEBUGV("HCI Disconnected\n");
        break;

    case HCI_EVENT_LE_META:
        // wait for connection complete
        if (hci_event_le_meta_get_subevent_code(packet) !=  HCI_SUBEVENT_LE_CONNECTION_COMPLETE) {
            break;
        }
        DEBUGV("HCI Connected\n");
        _hciConn = hci_subevent_le_connection_complete_get_connection_handle(packet);
        if (_smPair) {
            sm_request_pairing(_hciConn);
        } else {
            // query primary services - not used yet
            gatt_client_discover_primary_services(handle_gatt_client_event, _hciConn);
        }
        break;

    default:
        break;
    }
}
