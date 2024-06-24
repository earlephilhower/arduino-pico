/**
    Arduino Wrapper for BTstack
*/

#include <Arduino.h>
#include <SPI.h>

#ifdef __AVR__
#include <avr/wdt.h>
#endif

#if __arm__
//#include <Reset.h>  // also provides NVIC_SystemReset
#endif

#include "BTstackLib.h"

#include "btstack_memory.h"
#include "hal_tick.h"
#include "hal_cpu.h"
#include "hci_cmd.h"
#include "btstack_util.h"
#include "btstack_run_loop.h"
#include "btstack_event.h"
#include "btstack_run_loop_embedded.h"
#include "hci_transport.h"
#include "hci_transport_h4.h"

#include "ad_parser.h"
//#include "btstack_chipset_em9301.h"
#include "btstack_debug.h"
#include "gap.h"
#include "hci.h"
#include "hci_dump.h"
#include "hci_dump_embedded_stdout.h"
#include "l2cap.h"
#include "ble/att_db.h"
#include "ble/att_server.h"
#include "ble/att_db_util.h"
#include "ble/le_device_db.h"
#include "ble/sm.h"

// Pin 13 has an LED connected on most Arduino boards.
//#define PIN_LED 13

// prototypes
extern "C" void hal_uart_dma_process(void);

enum {
    SET_ADVERTISEMENT_PARAMS  = 1 << 0,
    SET_ADVERTISEMENT_DATA    = 1 << 1,
    SET_ADVERTISEMENT_ENABLED = 1 << 2,
};

typedef enum gattAction {
    gattActionWrite,
    gattActionSubscribe,
    gattActionUnsubscribe,
    gattActionServiceQuery,
    gattActionCharacteristicQuery,
    gattActionRead,
} gattAction_t;

static gattAction_t gattAction;

// btstack state
static int btstack_state;

static const uint8_t iBeaconAdvertisement01[] = { 0x02, 0x01 };
static const uint8_t iBeaconAdvertisement38[] = { 0x1a, 0xff, 0x4c, 0x00, 0x02, 0x15 };
static uint8_t   adv_data[31];
static uint16_t  adv_data_len = 0;
//static int gatt_is_characteristics_query;

static uint16_t le_peripheral_todos = 0;
static bool have_custom_addr;
static bd_addr_t public_bd_addr;

static btstack_timer_source_t connection_timer;

static void (*bleAdvertismentCallback)(BLEAdvertisement * bleAdvertisement) = NULL;
static void (*bleDeviceConnectedCallback)(BLEStatus status, BLEDevice * device) = NULL;
static void (*bleDeviceDisconnectedCallback)(BLEDevice * device) = NULL;
static void (*gattServiceDiscoveredCallback)(BLEStatus status, BLEDevice * device, BLEService * bleService) = NULL;
static void (*gattCharacteristicDiscoveredCallback)(BLEStatus status, BLEDevice * device, BLECharacteristic * characteristic) = NULL;
static void (*gattCharacteristicNotificationCallback)(BLEDevice * device, uint16_t value_handle, uint8_t* value, uint16_t length) = NULL;
static void (*gattCharacteristicIndicationCallback)(BLEDevice * device, uint16_t value_handle, uint8_t* value, uint16_t length) = NULL;
static void (*gattCharacteristicReadCallback)(BLEStatus status, BLEDevice * device, uint8_t * value, uint16_t length) = NULL;
static void (*gattCharacteristicWrittenCallback)(BLEStatus status, BLEDevice * device) = NULL;
static void (*gattCharacteristicSubscribedCallback)(BLEStatus status, BLEDevice * device) = NULL;
static void (*gattCharacteristicUnsubscribedCallback)(BLEStatus status, BLEDevice * device) = NULL;


// retarget printf to Serial
#ifdef ENERGIA
extern "C" int putchar(int c) {
    Serial.write((uint8_t)c);
    return c;
}
#else
#ifdef __AVR__
static FILE uartout = {0} ;
static int uart_putchar(char c, FILE *stream) {
    Serial.write(c);
    return 0;
}
#endif
// added for Arduino Zero. Arduino Due already has tis own _write(..) implementation
// in  /Users/mringwal/Library/Arduino15/packages/arduino/hardware/sam/1.6.4/cores/arduino/syscalls_sam3.c
#if defined(__SAMD21G18A__)
// #ifdef __arm__
extern "C" int _write(int file, char *ptr, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (ptr[i] == '\n') {
            Serial.write((uint8_t)'\r');
        }
        Serial.write((uint8_t)ptr[i]);
    }
    return i;
}
#endif
#endif

// HAL CPU Implementation
extern "C" void hal_cpu_disable_irqs(void) { }
extern "C" void hal_cpu_enable_irqs(void) { }
extern "C" void hal_cpu_enable_irqs_and_sleep(void) { }

//
static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void) channel;
    (void) size;
    hci_con_handle_t con_handle;

    switch (packet_type) {

    case HCI_EVENT_PACKET:
        switch (packet[0]) {

        case BTSTACK_EVENT_STATE:
            btstack_state = packet[2];
            // bt stack activated, get started
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                le_peripheral_todos |= SET_ADVERTISEMENT_PARAMS
                                       | SET_ADVERTISEMENT_DATA
                                       | SET_ADVERTISEMENT_ENABLED;
                bd_addr_t addr;
                gap_local_bd_addr(addr);
                printf("BTstack up and running at %s\n",  bd_addr_to_str(addr));
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            if (bleDeviceDisconnectedCallback) {
                con_handle = little_endian_read_16(packet, 3);
                BLEDevice device(con_handle);
                (*bleDeviceDisconnectedCallback)(&device);
            }
            le_peripheral_todos |= SET_ADVERTISEMENT_ENABLED;
            break;

        case GAP_EVENT_ADVERTISING_REPORT: {
            if (bleAdvertismentCallback) {
                BLEAdvertisement advertisement(packet);
                (*bleAdvertismentCallback)(&advertisement);
            }
            break;
        }

        case HCI_EVENT_LE_META:
            switch (packet[2]) {
            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                con_handle = little_endian_read_16(packet, 4);
                printf("Connection complete, con_handle 0x%04x\n", con_handle);
                btstack_run_loop_remove_timer(&connection_timer);
                if (!bleDeviceConnectedCallback) {
                    break;
                }
                if (packet[3]) {
                    (*bleDeviceConnectedCallback)(BLE_STATUS_CONNECTION_ERROR, NULL);
                } else {
                    BLEDevice device(con_handle);
                    (*bleDeviceConnectedCallback)(BLE_STATUS_OK, &device);
                }
                break;
            default:
                break;
            }
            break;
        }
    }
}

static void extract_service(gatt_client_service_t * service, uint8_t * packet) {
    service->start_group_handle = little_endian_read_16(packet, 4);
    service->end_group_handle   = little_endian_read_16(packet, 6);
    service->uuid16 = 0;
    reverse_128(&packet[8], service->uuid128);
    if (uuid_has_bluetooth_prefix(service->uuid128)) {
        service->uuid16 = big_endian_read_32(service->uuid128, 0);
    }
}

static void extract_characteristic(gatt_client_characteristic_t * characteristic, uint8_t * packet) {
    characteristic->start_handle = little_endian_read_16(packet, 4);
    characteristic->value_handle = little_endian_read_16(packet, 6);
    characteristic->end_handle =   little_endian_read_16(packet, 8);
    characteristic->properties =   little_endian_read_16(packet, 10);
    characteristic->uuid16 = 0;
    reverse_128(&packet[12], characteristic->uuid128);
    if (uuid_has_bluetooth_prefix(characteristic->uuid128)) {
        characteristic->uuid16 = big_endian_read_32(characteristic->uuid128, 0);
    }
}

static void gatt_client_callback(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t size) {
    (void) channel;
    (void) packet_type;
    (void) size;
    // if (hci) event is not 4-byte aligned, event->handle causes crash
    // workaround: check event type, assuming GATT event types are contagious
    if (packet[0] < GATT_EVENT_QUERY_COMPLETE) {
        return;
    }
    if (packet[0] > GATT_EVENT_MTU) {
        return;
    }

    hci_con_handle_t con_handle = little_endian_read_16(packet, 2);
    uint8_t   status;
    uint8_t * value;
    uint16_t  value_handle;
    uint16_t  value_length;

    BLEDevice device(con_handle);
    switch (hci_event_packet_get_type(packet)) {
    case GATT_EVENT_SERVICE_QUERY_RESULT:
        if (gattServiceDiscoveredCallback) {
            gatt_client_service_t service;
            extract_service(&service, packet);
            BLEService bleService(service);
            (*gattServiceDiscoveredCallback)(BLE_STATUS_OK, &device, &bleService);
        }
        break;
    case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
        if (gattCharacteristicDiscoveredCallback) {
            gatt_client_characteristic_t characteristic;
            extract_characteristic(&characteristic, packet);
            BLECharacteristic bleCharacteristic(characteristic);
            (*gattCharacteristicDiscoveredCallback)(BLE_STATUS_OK, &device, &bleCharacteristic);
        }
        break;
    case GATT_EVENT_QUERY_COMPLETE:
        status = little_endian_read_16(packet, 4);
        switch (gattAction) {
        case gattActionWrite:
            if (gattCharacteristicWrittenCallback) {
                gattCharacteristicWrittenCallback(status ? BLE_STATUS_OTHER_ERROR : BLE_STATUS_OK, &device);
            }
            break;
        case gattActionSubscribe:
            if (gattCharacteristicSubscribedCallback) {
                gattCharacteristicSubscribedCallback(status ? BLE_STATUS_OTHER_ERROR : BLE_STATUS_OK, &device);
            }
            break;
        case gattActionUnsubscribe:
            if (gattCharacteristicUnsubscribedCallback) {
                gattCharacteristicUnsubscribedCallback(status ? BLE_STATUS_OTHER_ERROR : BLE_STATUS_OK, &device);
            }
            break;
        case gattActionServiceQuery:
            if (gattServiceDiscoveredCallback) {
                gattServiceDiscoveredCallback(BLE_STATUS_DONE, &device, NULL);
            }
            break;
        case gattActionCharacteristicQuery:
            if (gattCharacteristicDiscoveredCallback) {
                gattCharacteristicDiscoveredCallback(BLE_STATUS_DONE, &device, NULL);
            }
            break;
        default:
            break;
        };
        break;
    case GATT_EVENT_NOTIFICATION:
        if (gattCharacteristicNotificationCallback) {
            value_handle = little_endian_read_16(packet, 4);
            value_length = little_endian_read_16(packet, 6);
            value = &packet[8];
            (*gattCharacteristicNotificationCallback)(&device, value_handle, value, value_length);
        }
        break;
    case GATT_EVENT_INDICATION:
        if (gattCharacteristicIndicationCallback) {
            value_handle = little_endian_read_16(packet, 4);
            value_length = little_endian_read_16(packet, 6);
            value = &packet[8];
            (*gattCharacteristicIndicationCallback)(&device, value_handle, value, value_length);
        }
        break;
    case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT:
        if (gattCharacteristicReadCallback) {
            value_handle = little_endian_read_16(packet, 4);
            value_length = little_endian_read_16(packet, 6);
            value = &packet[8];
            (*gattCharacteristicReadCallback)(BLE_STATUS_OK, &device, value, value_length);
        }
        break;
    default:
        break;
    }
}

static void connection_timeout_handler(btstack_timer_source_t * timer) {
    (void) timer;
    // log_info("Cancel outgoing connection");
    gap_connect_cancel();
    if (!bleDeviceConnectedCallback) {
        return;
    }
    (*bleDeviceConnectedCallback)(BLE_STATUS_CONNECTION_TIMEOUT, NULL);  // page timeout 0x04
}

//

/// UUID class
UUID::UUID(void) {
    memset(uuid, 0, 16);
}

UUID::UUID(const uint8_t uuid[16]) {
    memcpy(this->uuid, uuid, 16);
}

UUID::UUID(const char * uuidStr) {
    memset(uuid, 0, 16);
    int len = strlen(uuidStr);
    if (len <= 4) {
        // Handle 4 Bytes HEX
        unsigned int uuid16;
        int result = sscanf((char *) uuidStr, "%x", &uuid16);
        if (result == 1) {
            uuid_add_bluetooth_prefix(uuid, uuid16);
        }
        return;
    }

    // quick UUID parser, ignoring dashes
    int i = 0;
    int data = 0;
    int have_nibble = 0;
    while (*uuidStr && i < 16) {
        const char c = *uuidStr++;
        if (c == '-') {
            continue;
        }
        data = data << 4 | nibble_for_char(c);
        if (!have_nibble) {
            have_nibble = 1;
            continue;
        }
        uuid[i++] = data;
        data = 0;
        have_nibble = 0;
    }
}

const uint8_t * UUID::getUuid(void) const {
    return uuid;
}

static char uuid16_buffer[5];
const char * UUID::getUuidString() const {
    // TODO: fix uuid_has_bluetooth_prefix call to use const
    if (uuid_has_bluetooth_prefix((uint8_t*)uuid)) {
        sprintf(uuid16_buffer, "%04x", (uint16_t) big_endian_read_32(uuid, 0));
        return uuid16_buffer;
    }  else {
        // TODO: fix uuid128_to_str
        return uuid128_to_str((uint8_t*)uuid);
    }
}

const char * UUID::getUuid128String() const {
    return uuid128_to_str((uint8_t*)uuid);
}

bool UUID::matches(UUID *other)        const {
    return memcmp(this->uuid, other->uuid, 16) == 0;
}


// BD_ADDR class
BD_ADDR::BD_ADDR(void) {
}

BD_ADDR::BD_ADDR(const char * address_string, BD_ADDR_TYPE address_type) : address_type(address_type) {
    int processed = sscanf(address_string, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", address, address + 1,
                           address + 2, address + 3, address + 4, address + 5);
    if (processed != 6) { // Set address to zeroes if we did not get six bytes back.
        for (int i = 0; i < 6; i++) {
            address[i] = 0;
        }
    }
}

BD_ADDR::BD_ADDR(const uint8_t address[6], BD_ADDR_TYPE address_type) : address_type(address_type) {
    memcpy(this->address, address, 6);
}

const uint8_t * BD_ADDR::getAddress(void) {
    return address;
}

const char * BD_ADDR::getAddressString(void) {
    return bd_addr_to_str(address);
}

BD_ADDR_TYPE BD_ADDR::getAddressType(void) {
    return address_type;
}


BLEAdvertisement::BLEAdvertisement(uint8_t * event_packet) :
    advertising_event_type(event_packet[2]),
    rssi(event_packet[10]),
    data_length(event_packet[11]),
    iBeacon_UUID(NULL) {
    bd_addr_t addr;
    reverse_bd_addr(&event_packet[4], addr);
    bd_addr = BD_ADDR(addr, (BD_ADDR_TYPE)event_packet[3]);
    memcpy(data, &event_packet[12], LE_ADVERTISING_DATA_SIZE);
}

BLEAdvertisement::~BLEAdvertisement() {
    if (iBeacon_UUID) {
        delete (iBeacon_UUID);
    }
}

const uint8_t * BLEAdvertisement::getAdvData(void) {
    return data;
}

BD_ADDR * BLEAdvertisement::getBdAddr(void) {
    return &bd_addr;
}

int BLEAdvertisement::getRssi(void) {
    return rssi > 127 ? rssi - 256 : rssi;
}


bool BLEAdvertisement::containsService(UUID * service) {
    return ad_data_contains_uuid128(data_length, data, (uint8_t*) service->getUuid());
}

bool BLEAdvertisement::nameHasPrefix(const char * name_prefix) {
    ad_context_t context;
    int name_prefix_len = strlen(name_prefix);
    for (ad_iterator_init(&context, data_length, data) ; ad_iterator_has_more(&context) ; ad_iterator_next(&context)) {
        uint8_t data_type = ad_iterator_get_data_type(&context);
        uint8_t data_len  = ad_iterator_get_data_len(&context);
        const uint8_t * data    = ad_iterator_get_data(&context);
        int compare_len = name_prefix_len;
        switch (data_type) {
        case 8: // shortented local name
        case 9: // complete local name
            if (compare_len > data_len) {
                compare_len = data_len;
            }
            if (strncmp(name_prefix, (const char*) data, compare_len) == 0) {
                return true;
            }
            break;
        default:
            break;
        }
    }
    return false;
};

bool BLEAdvertisement::isIBeacon(void) {
    return ((memcmp(iBeaconAdvertisement01,  data,    sizeof(iBeaconAdvertisement01)) == 0)
            && (memcmp(iBeaconAdvertisement38, &data[3], sizeof(iBeaconAdvertisement38)) == 0));
}

const UUID * BLEAdvertisement::getIBeaconUUID(void) {
    if (!iBeacon_UUID) {
        iBeacon_UUID = new UUID(&data[9]);
    }
    return iBeacon_UUID;
};
uint16_t BLEAdvertisement::getIBeaconMajorID(void) {
    return big_endian_read_16(data, 25);
};
uint16_t BLEAdvertisement::getIBecaonMinorID(void) {
    return big_endian_read_16(data, 27);
};
uint8_t BLEAdvertisement::getiBeaconMeasuredPower(void) {
    return data[29];
}


BLECharacteristic::BLECharacteristic(void) {
}

BLECharacteristic::BLECharacteristic(gatt_client_characteristic_t characteristic)
    : characteristic(characteristic), uuid(characteristic.uuid128) {
}

const UUID * BLECharacteristic::getUUID(void) {
    return &uuid;
}

bool BLECharacteristic::matches(UUID * uuid) {
    return this->uuid.matches(uuid);
}

bool BLECharacteristic::isValueHandle(uint16_t value_handle) {
    return characteristic.value_handle == value_handle;
}

const gatt_client_characteristic_t * BLECharacteristic::getCharacteristic(void) {
    return &characteristic;
}

gatt_client_notification_t *BLECharacteristic::getNotifier() {
    return &notify;
}


BLEService::BLEService(void) {
}

BLEService::BLEService(gatt_client_service_t service)
    : service(service), uuid(service.uuid128) {
}

const UUID * BLEService::getUUID(void) {
    return &uuid;
}

bool BLEService::matches(UUID * uuid) {
    return this->uuid.matches(uuid);
}

const gatt_client_service_t * BLEService::getService(void) {
    return &service;
}

// discovery of services and characteristics
BLEDevice::BLEDevice(void) {
}
BLEDevice::BLEDevice(hci_con_handle_t handle)
    : handle(handle) {
}
uint16_t BLEDevice::getHandle(void) {
    return handle;
}
int BLEDevice::discoverGATTServices(void) {
    return BTstack.discoverGATTServices(this);
}
int BLEDevice::discoverCharacteristicsForService(BLEService * service) {
    return BTstack.discoverCharacteristicsForService(this, service);
}
int BLEDevice::readCharacteristic(BLECharacteristic * characteristic) {
    return BTstack.readCharacteristic(this, characteristic);
}
int BLEDevice::writeCharacteristic(BLECharacteristic * characteristic, uint8_t * data, uint16_t size) {
    return BTstack.writeCharacteristic(this, characteristic, data, size);
}
int BLEDevice::writeCharacteristicWithoutResponse(BLECharacteristic * characteristic, uint8_t * data, uint16_t size) {
    return BTstack.writeCharacteristicWithoutResponse(this, characteristic, data, size);
}
int BLEDevice::subscribeForNotifications(BLECharacteristic * characteristic) {
    return BTstack.subscribeForNotifications(this, characteristic);
}
int BLEDevice::unsubscribeFromNotifications(BLECharacteristic * characteristic) {
    return BTstack.unsubscribeFromNotifications(this, characteristic);
}
int BLEDevice::subscribeForIndications(BLECharacteristic * characteristic) {
    return BTstack.subscribeForIndications(this, characteristic);
}
int BLEDevice::unsubscribeFromIndications(BLECharacteristic * characteristic) {
    return BTstack.unsubscribeFromIndications(this, characteristic);
}



static uint16_t (*gattReadCallback)(uint16_t characteristic_id, uint8_t * buffer, uint16_t buffer_size);
static int (*gattWriteCallback)(uint16_t characteristic_id, uint8_t *buffer, uint16_t buffer_size);

// ATT Client Read Callback for Dynamic Data
// - if buffer == NULL, don't copy data, just return size of value
// - if buffer != NULL, copy data and return number bytes copied
// @param offset defines start of attribute value
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size) {
    (void) con_handle;
    (void) offset;
    if (gattReadCallback) {
        return gattReadCallback(att_handle, buffer, buffer_size);
    }
    return 0;
}
/* LISTING_END */


/*
    @section ATT Write

    @text The only valid ATT write in this example is to the Client Characteristic Configuration, which configures notification
    and indication. If the ATT handle matches the client configuration handle, the new configuration value is stored and used
    in the heartbeat handler to decide if a new value should be sent. See Listing attWrite.
*/

/* LISTING_START(attWrite): ATT Write */
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    (void) con_handle;
    (void) transaction_mode;
    (void) offset;
    if (gattWriteCallback) {
        gattWriteCallback(att_handle, buffer, buffer_size);
    }
    return 0;
}



BTstackManager::BTstackManager(void) {
    // client_packet_handler = NULL;
    have_custom_addr = false;
    // reset handler
    bleAdvertismentCallback = NULL;
    bleDeviceConnectedCallback = NULL;
    bleDeviceDisconnectedCallback = NULL;
    gattServiceDiscoveredCallback = NULL;
    gattCharacteristicDiscoveredCallback = NULL;
    gattCharacteristicNotificationCallback = NULL;

    att_db_util_init();

    // disable LOG_INFO messages
    hci_dump_enable_log_level(HCI_DUMP_LOG_LEVEL_INFO, 0);

#ifdef __AVR__
    // configure stdout to go via Serial
    fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartout;
#endif
}

void BTstackManager::setBLEAdvertisementCallback(void (*callback)(BLEAdvertisement * bleAdvertisement)) {
    bleAdvertismentCallback = callback;
}
void BTstackManager::setBLEDeviceConnectedCallback(void (*callback)(BLEStatus status, BLEDevice * device)) {
    bleDeviceConnectedCallback = callback;
}
void BTstackManager::setBLEDeviceDisconnectedCallback(void (*callback)(BLEDevice * device)) {
    bleDeviceDisconnectedCallback = callback;
}
void BTstackManager::setGATTServiceDiscoveredCallback(void (*callback)(BLEStatus status, BLEDevice * device, BLEService * bleService)) {
    gattServiceDiscoveredCallback = callback;
}
void BTstackManager::setGATTCharacteristicDiscoveredCallback(void (*callback)(BLEStatus status, BLEDevice * device, BLECharacteristic * characteristic)) {
    gattCharacteristicDiscoveredCallback = callback;
}
void BTstackManager::setGATTCharacteristicNotificationCallback(void (*callback)(BLEDevice * device, uint16_t value_handle, uint8_t* value, uint16_t length)) {
    gattCharacteristicNotificationCallback = callback;
}
void BTstackManager::setGATTCharacteristicIndicationCallback(void (*callback)(BLEDevice * device, uint16_t value_handle, uint8_t* value, uint16_t length)) {
    gattCharacteristicIndicationCallback = callback;
}
void BTstackManager::setGATTCharacteristicReadCallback(void (*callback)(BLEStatus status, BLEDevice * device, uint8_t * value, uint16_t length)) {
    gattCharacteristicReadCallback = callback;
}
void BTstackManager::setGATTCharacteristicWrittenCallback(void (*callback)(BLEStatus status, BLEDevice * device)) {
    gattCharacteristicWrittenCallback = callback;
}
void BTstackManager::setGATTCharacteristicSubscribedCallback(void (*callback)(BLEStatus status, BLEDevice * device)) {
    gattCharacteristicSubscribedCallback = callback;
}
void BTstackManager::setGATTCharacteristicUnsubscribedCallback(void (*callback)(BLEStatus status, BLEDevice * device)) {
    gattCharacteristicUnsubscribedCallback = callback;
}

int BTstackManager::discoverGATTServices(BLEDevice * device) {
    gattAction = gattActionServiceQuery;
    return gatt_client_discover_primary_services(gatt_client_callback, device->getHandle());
}
int BTstackManager::discoverCharacteristicsForService(BLEDevice * device, BLEService * service) {
    gattAction = gattActionCharacteristicQuery;
    return gatt_client_discover_characteristics_for_service(gatt_client_callback, device->getHandle(), (gatt_client_service_t*) service->getService());
}
int  BTstackManager::readCharacteristic(BLEDevice * device, BLECharacteristic * characteristic) {
    return gatt_client_read_value_of_characteristic(gatt_client_callback, device->getHandle(), (gatt_client_characteristic_t*) characteristic->getCharacteristic());
}
int  BTstackManager::writeCharacteristic(BLEDevice * device, BLECharacteristic * characteristic, uint8_t * data, uint16_t size) {
    gattAction = gattActionWrite;
    return gatt_client_write_value_of_characteristic(gatt_client_callback, device->getHandle(), characteristic->getCharacteristic()->value_handle,
            size, data);
}
int  BTstackManager::writeCharacteristicWithoutResponse(BLEDevice * device, BLECharacteristic * characteristic, uint8_t * data, uint16_t size) {
    return gatt_client_write_value_of_characteristic_without_response(device->getHandle(), characteristic->getCharacteristic()->value_handle,
            size, data);
}
int BTstackManager::subscribeForNotifications(BLEDevice * device, BLECharacteristic * characteristic) {
    gattAction = gattActionSubscribe;
    gatt_client_listen_for_characteristic_value_updates(characteristic->getNotifier(), gatt_client_callback,  device->getHandle(), (gatt_client_characteristic_t*) characteristic->getCharacteristic());
    return gatt_client_write_client_characteristic_configuration(gatt_client_callback, device->getHandle(), (gatt_client_characteristic_t*) characteristic->getCharacteristic(),
            GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
}
int BTstackManager::subscribeForIndications(BLEDevice * device, BLECharacteristic * characteristic) {
    gattAction = gattActionSubscribe;
    return gatt_client_write_client_characteristic_configuration(gatt_client_callback, device->getHandle(), (gatt_client_characteristic_t*) characteristic->getCharacteristic(),
            GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_INDICATION);
}
int BTstackManager::unsubscribeFromNotifications(BLEDevice * device, BLECharacteristic * characteristic) {
    gattAction = gattActionUnsubscribe;
    gatt_client_stop_listening_for_characteristic_value_updates(characteristic->getNotifier());
    return gatt_client_write_client_characteristic_configuration(gatt_client_callback, device->getHandle(), (gatt_client_characteristic_t*) characteristic->getCharacteristic(),
            GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NONE);
}
int BTstackManager::unsubscribeFromIndications(BLEDevice * device, BLECharacteristic * characteristic) {
    gattAction = gattActionUnsubscribe;
    return gatt_client_write_client_characteristic_configuration(gatt_client_callback, device->getHandle(), (gatt_client_characteristic_t*) characteristic->getCharacteristic(),
            GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NONE);
}
void BTstackManager::bleConnect(BLEAdvertisement * advertisement, int timeout_ms) {
    bleConnect(advertisement->getBdAddr(), timeout_ms);
}
void BTstackManager::bleConnect(BD_ADDR * address, int timeout_ms) {
    bleConnect(address->getAddressType(), address->getAddress(), timeout_ms);
}
void BTstackManager::bleConnect(BD_ADDR_TYPE address_type, const char * address, int timeout_ms) {
    (void) address_type;
    (void) address;
    (void) timeout_ms;
    // TODO: implement
    // log_error("BTstackManager::bleConnect(BD_ADDR_TYPE address_type, const char * address, int timeout_ms) not implemented");
}
void BTstackManager::bleConnect(BD_ADDR_TYPE address_type, const uint8_t address[6], int timeout_ms) {
    gap_connect((uint8_t*)address, (bd_addr_type_t) address_type);
    if (!timeout_ms) {
        return;
    }
    btstack_run_loop_set_timer(&connection_timer, timeout_ms);
    btstack_run_loop_set_timer_handler(&connection_timer, connection_timeout_handler);
    btstack_run_loop_add_timer(&connection_timer);
}

void BTstackManager::bleDisconnect(BLEDevice * device) {
    btstack_run_loop_remove_timer(&connection_timer);
    gap_disconnect(device->getHandle());
}

void BTstackManager::setPublicBdAddr(bd_addr_t addr) {
    have_custom_addr = true;
    memcpy(public_bd_addr, addr, 6);
}

void bluetooth_hardware_error(uint8_t error) {
    printf("Bluetooth Hardware Error event 0x%02x. Restarting...\n\n\n", error);
#ifdef __AVR__
    wdt_enable(WDTO_15MS);
    // wait for watchdog to trigger
#endif

#ifdef __arm__
    //    NVIC_SystemReset();
#endif
    while (1);
}
#if 0
static hci_transport_config_uart_t config = {
    HCI_TRANSPORT_CONFIG_UART,
    115200,
    0,  // main baudrate
    1,  // flow control
    NULL,
};
#endif
static btstack_packet_callback_registration_t hci_event_callback_registration;

void BTstackManager::setup(void) {
    setup("BTstack LE Shield");
}

void BTstackManager::setup(const char * name) {

    //#ifdef PIN_LED
    //    pinMode(PIN_LED, OUTPUT);
    //#endif

    //    printf("BTstackManager::setup()\n");
#if 0
    btstack_memory_init();
    btstack_run_loop_init(btstack_run_loop_embedded_get_instance());

    const hci_transport_t * transport = hci_transport_h4_instance(btstack_uart_block_embedded_instance());
    hci_init(transport, (void*) &config);
    hci_set_chipset(btstack_chipset_em9301_instance());

    if (have_custom_addr) {
        hci_set_bd_addr(public_bd_addr);
    }

    hci_set_hardware_error_callback(&bluetooth_hardware_error);

#endif
    // inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    l2cap_init();

    sm_init();

    att_server_init(att_db_util_get_address(), att_read_callback, att_write_callback);

    gatt_client_init();

    // setup advertisements params
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);

    // setup advertisements data
    int pos = 0;
    const uint8_t flags[] = { 0x02, 0x01, 0x02 };
    memcpy(&adv_data[pos], flags, sizeof(flags));
    pos += sizeof(flags);
    adv_data[pos++] = strlen(name) + 1;
    adv_data[pos++] = 0x09;
    memcpy(&adv_data[pos], name, strlen(name));
    pos += strlen(name);
    adv_data_len = pos;
    gap_advertisements_set_data(adv_data_len, adv_data);

    // turn on!
    btstack_state = 0;
    hci_power_control(HCI_POWER_ON);
}

void BTstackManager::enablePacketLogger(void) {
    hci_dump_init(hci_dump_embedded_stdout_get_instance());
}

void BTstackManager::enableDebugLogger() {
    // enable LOG_INFO messages
    hci_dump_enable_log_level(HCI_DUMP_LOG_LEVEL_INFO, 1);
}


void BTstackManager::loop(void) {
    // process data from/to Bluetooth module
    //    hal_uart_dma_process();
    // BTstack Run Loop
    //    btstack_run_loop_embedded_execute_once();
}

void BTstackManager::bleStartScanning(void) {
    //    printf("Start scanning\n");
    gap_start_scan();
}
void BTstackManager::bleStopScanning(void) {
    gap_stop_scan();
}

void BTstackManager::setGATTCharacteristicRead(uint16_t (*cb)(uint16_t characteristic_id, uint8_t * buffer, uint16_t buffer_size)) {
    gattReadCallback = cb;
}
void BTstackManager::setGATTCharacteristicWrite(int (*cb)(uint16_t characteristic_id, uint8_t *buffer, uint16_t buffer_size)) {
    gattWriteCallback = cb;
}
void BTstackManager::addGATTService(UUID * uuid) {
    att_db_util_add_service_uuid128((uint8_t*)uuid->getUuid());
}
uint16_t BTstackManager::addGATTCharacteristic(UUID * uuid, uint16_t flags, const char * text) {
    return att_db_util_add_characteristic_uuid128((uint8_t*)uuid->getUuid(), flags, ATT_SECURITY_NONE, ATT_SECURITY_NONE, (uint8_t*)text, strlen(text));
}
uint16_t BTstackManager::addGATTCharacteristic(UUID * uuid, uint16_t flags, uint8_t * data, uint16_t data_len) {
    return att_db_util_add_characteristic_uuid128((uint8_t*)uuid->getUuid(), flags, ATT_SECURITY_NONE, ATT_SECURITY_NONE, data, data_len);
}
uint16_t BTstackManager::addGATTCharacteristicDynamic(UUID * uuid, uint16_t flags, uint16_t characteristic_id) {
    (void) characteristic_id;
    return att_db_util_add_characteristic_uuid128((uint8_t*)uuid->getUuid(), flags | ATT_PROPERTY_DYNAMIC, ATT_SECURITY_NONE, ATT_SECURITY_NONE, NULL, 0);
}
void BTstackManager::setAdvData(uint16_t adv_data_len, const uint8_t * adv_data) {
    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
}
void BTstackManager::setScanData(uint16_t scan_data_len, const uint8_t * scan_data) {
    gap_scan_response_set_data(scan_data_len, (uint8_t*) scan_data);
}
void BTstackManager::startAdvertising() {
    gap_advertisements_enable(1);
}
void BTstackManager::stopAdvertising() {
    gap_advertisements_enable(0);
}
void BTstackManager::iBeaconConfigure(UUID * uuid, uint16_t major_id, uint16_t minor_id, uint8_t measured_power) {
    memcpy(adv_data, iBeaconAdvertisement01,  sizeof(iBeaconAdvertisement01));
    adv_data[2] = 0x06;
    memcpy(&adv_data[3], iBeaconAdvertisement38, sizeof(iBeaconAdvertisement38));
    memcpy(&adv_data[9], uuid->getUuid(), 16);
    big_endian_store_16(adv_data, 25, major_id);
    big_endian_store_16(adv_data, 27, minor_id);
    adv_data[29] = measured_power;
    adv_data_len = 30;
    gap_advertisements_set_data(adv_data_len, adv_data);
}
// 02 01 06 1A FF 4C 00 02 15 -- F8 97 17 7B AE E8 47 67 8E CC CC 69 4F D5 FC EE -- 12 67 00 02 00 00
// 02 01 06 1a ff 4c 00 02 15 -- FB 0B 57 A2 82 28 44 CD 91 3A 94 A1 22 BA 12 06 -- 00 01 00 02 D1 00


BTstackManager BTstack;

