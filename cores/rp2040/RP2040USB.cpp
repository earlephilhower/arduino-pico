/*
    Shared USB for the Raspberry Pi Pico RP2040
    Allows for multiple endpoints to share the USB controller

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#if !defined(USE_TINYUSB) && !defined(NO_USB)

#include <Arduino.h>
#include "CoreMutex.h"
#include "RP2040USB.h"

#include <tusb.h>
#include <class/hid/hid_device.h>
#include <pico/time.h>
#include <hardware/irq.h>
#include <pico/mutex.h>
#include <pico/unique_id.h>
#include <pico/usb_reset_interface.h>
#include <hardware/watchdog.h>

#ifdef __FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#endif

RP2040USB USB;

// USB processing will be a periodic timer task
#define USB_TASK_INTERVAL 1000

#ifndef USBD_VID
#define USBD_VID (0x2E8A) // Raspberry Pi
#endif

#ifndef USBD_PID
#define USBD_PID (0x000a) // Raspberry Pi Pico SDK CDC
#endif

#define TUD_RPI_RESET_DESCRIPTOR(_itfnum, _stridx) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 0, TUSB_CLASS_VENDOR_SPECIFIC, RESET_INTERFACE_SUBCLASS, RESET_INTERFACE_PROTOCOL, _stridx,

// We can't use non-trivial variables to hold the hid, interface, or string lists.  The global
// initialization where things like the global Keyboard may be called before the non-trivial
// objects (i.e. no std::vector).

// Either a USB interface or HID device descriptor, kept in a linked list
typedef struct Entry {
    const uint8_t *descriptor;
    unsigned int len        : 12;
    unsigned int interfaces : 4;
    unsigned int order      : 18;
    unsigned int localid    : 6;
    uint32_t mask;
    struct Entry *next;
} Entry;

static Entry *_hids = nullptr;
static Entry *_interfaces = nullptr;

// USB strings kept in a list of pointers.  Can't use std::vector again because of
// CRT init non-ordering.
static const char **usbd_desc_str;
static uint8_t usbd_desc_str_cnt = 0;
static uint8_t usbd_desc_str_alloc = 0;

// HID report
static unsigned int __hid_interface = (unsigned int) -1;
static uint8_t __hid_endpoint = 0;
static int      __hid_report_len = 0;
static uint8_t *__hid_report     = nullptr;

// Global USB descriptor
static uint8_t *usbd_desc_cfg = nullptr;
static int usbd_desc_cfg_len = 0;

#ifdef ENABLE_PICOTOOL_USB
static uint8_t _picotool_itf_num;
#endif
int usb_hid_poll_interval __attribute__((weak)) = 10;


// Available bitmask for endpoints, can never be EP 0
static uint32_t _endpointIn = 0xfffffffe;
static uint32_t _endpointOut = 0xfffffffe;

// GCC doesn't seem to have builtin_ffs here
static int ffs(uint32_t v) {
    for (auto i = 0; i < 32; i++) {
        if (v & (1 << i)) {
            return i;
        }
    }
    return 0;
}

uint8_t RP2040USB::registerEndpointIn() {
    if (!_endpointIn) {
        return 0; // ERROR, out of EPs
    }
    int firstFree = ffs(_endpointIn);
    _endpointIn &= ~(1 << firstFree);
    return 0x80 + firstFree;
}

void RP2040USB::unregisterEndpointIn(int ep) {
    _endpointIn |= 1 << ep;
}

uint8_t RP2040USB::registerEndpointOut() {
    if (!_endpointOut) {
        return 0; // ERROR, out of EPs
    }
    int firstFree = ffs(_endpointOut);
    _endpointOut &= ~(1 << firstFree);
    return firstFree;
}

void RP2040USB::unregisterEndpointOut(int ep) {
    _endpointOut |= (1 << (ep - 0x80));
}

static uint8_t AddEntry(Entry **head, int interfaces, const uint8_t *descriptor, size_t len, int ordering, uint32_t vidMask) {
    static uint8_t id = 1;

    Entry *n = (Entry *)malloc(sizeof(Entry));
    assert(n);
    n->descriptor = descriptor;
    n->len = len;
    n->interfaces = interfaces;
    n->order = ordering;
    n->localid = id++;
    n->mask = vidMask;
    n->next = nullptr;

    // Go down list until we hit the end or an entry with ordering >= our level
    Entry *prev = nullptr;
    Entry *cur = *head;
    while (cur && (ordering > cur->order)) {
        prev = cur;
        cur = cur->next;
    }
    if (!prev) {
        n->next = *head;
        *head = n;
    } else if (!cur) {
        prev->next = n;
    } else {
        n->next = cur;
        prev->next = n;
    }
    return n->localid;
}

static void RemoveEntry(Entry **head, unsigned int localid) {
    Entry *prev = nullptr;
    Entry *cur = *head;
    while (cur && cur->localid != localid) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) {
        // Not found, just exit
        return;
    }
    if (cur == *head) {
        auto p = cur->next;
        free(*head);
        *head = p;
    } else {
        prev->next = cur->next;
        free(cur);
    }
}

// Find the index (HID report ID or USB interface) of a given localid
static unsigned int usbFindID(Entry *head, unsigned int localid) {
    unsigned int x = 0;
    while (head && head->localid != localid) {
        head = head->next;
        x++;
    }
    assert(head);
    return x;
}

uint8_t RP2040USB::findHIDReportID(unsigned int localid) {
    return usbFindID(_hids, localid) + 1; // HID reports start at 1
}

uint8_t RP2040USB::findInterfaceID(unsigned int localid) {
    return usbFindID(_interfaces, localid);
}

// Called by a HID device to register a report.  Returns the *local* ID which must be mapped to the HID report ID
uint8_t RP2040USB::registerHIDDevice(const uint8_t *descriptor, size_t len, int ordering, uint32_t vidMask) {
    return AddEntry(&_hids, 0, descriptor, len, ordering, vidMask);
}

void RP2040USB::unregisterHIDDevice(unsigned int localid) {
    RemoveEntry(&_hids, localid);
}

// Called by an object at global init time to add a new interface (non-HID, like CDC or Picotool)
uint8_t RP2040USB::registerInterface(int interfaces, const uint8_t *descriptor, size_t len, int ordering, uint32_t vidMask) {
    return AddEntry(&_interfaces, interfaces, descriptor, len, ordering, vidMask);
}

void RP2040USB::unregisterInterface(unsigned int localid) {
    RemoveEntry(&_interfaces, localid);
}

uint8_t RP2040USB::registerString(const char *str) {
    if (usbd_desc_str_alloc <= usbd_desc_str_cnt) {
        usbd_desc_str_alloc += 4;
        usbd_desc_str = (const char **)realloc(usbd_desc_str, usbd_desc_str_alloc * sizeof(usbd_desc_str[0]));
    }

    if (!usbd_desc_str_cnt) {
        usbd_desc_str[0] = "";
        usbd_desc_str_cnt++;
    }
    // Don't re-add strings that already exist
    for (size_t i = 0; i < usbd_desc_str_cnt; i++) {
        if (!strcmp(str, usbd_desc_str[i])) {
            return i;
        }
    }
    usbd_desc_str[usbd_desc_str_cnt] = str;
    return usbd_desc_str_cnt++;
}

static uint16_t _forceVID = 0;
static uint16_t _forcePID = 0;
void RP2040USB::setVIDPID(uint16_t vid, uint16_t pid) {
    _forceVID = vid;
    _forcePID = pid;
}

static uint8_t _forceManuf = 0;
static uint8_t _forceProd = 0;
static uint8_t _forceSerial = 0;
void RP2040USB::setManufacturer(const char *str) {
    _forceManuf = USB.registerString(str);
}
void RP2040USB::setProduct(const char *str) {
    _forceProd = USB.registerString(str);
}
void RP2040USB::setSerialNumber(const char *str) {
    _forceSerial = USB.registerString(str);
}

static tusb_desc_device_t usbd_desc_device;
const uint8_t *tud_descriptor_device_cb(void) {
    static char idString[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 3 + 1];
    if (!idString[0]) {
        pico_get_unique_board_id_string(idString, sizeof(idString));
    }

    usbd_desc_device = {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0200,
        .bDeviceClass = 0,
        .bDeviceSubClass = 0,
        .bDeviceProtocol = 0,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
        .idVendor = _forceVID ? _forceVID : (uint16_t)USBD_VID,
        .idProduct = _forcePID ? _forcePID : (uint16_t)USBD_PID,
        .bcdDevice = 0x0100,
        .iManufacturer = _forceManuf ? _forceManuf : USB.registerString(USB_MANUFACTURER),
        .iProduct = _forceProd ? _forceProd : USB.registerString(USB_PRODUCT),
        .iSerialNumber = _forceSerial ? _forceSerial : USB.registerString(idString),
        .bNumConfigurations = 1
    };

    // Handle any inversions from the sub-devices, if we're not forcing things
    if (!_forcePID) {
        Entry *h = _hids;
        while (h) {
            usbd_desc_device.idProduct ^= h->mask;
            h = h->next;
        }
        h = _interfaces;
        while (h) {
            usbd_desc_device.idProduct ^= h->mask;
            h = h->next;
        }
    }

    return (const uint8_t *)&usbd_desc_device;
}


static uint8_t *GetDescHIDReport(int *len) {
    if (len) {
        *len = __hid_report_len;
    }
    return __hid_report;
}

void __SetupDescHIDReport() {
    __hid_report_len = 0;
    Entry *h = _hids;
    while (h) {
        __hid_report_len += h->len;
        h = h->next;
    }

    // No HID used at all
    if (__hid_report_len == 0) {
        __hid_report = nullptr;
        return;
    }

    // Allocate the "real" HID report descriptor
    __hid_report = (uint8_t *)malloc(__hid_report_len);
    assert(__hid_report);

    // Now copy the descriptors
    uint8_t *p = __hid_report;
    uint8_t id = 1;
    h = _hids;
    while (h) {
        memcpy(p, h->descriptor, h->len);
        // Need to update the report ID, a 2-byte value
        char buff[] = { HID_REPORT_ID(id) };
        memcpy(p + 6, buff, sizeof(buff));
        p += h->len;
        id++;
        h = h->next;
    }
}

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return GetDescHIDReport(nullptr);
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return usbd_desc_cfg;
}

// Build the binary image of the complete USB descriptor
// Note that we can add stack-allocated descriptors here because we know
// we're going to use them before the function exits and they'll not be
// needed ever again
void __SetupUSBDescriptor() {
    uint8_t interface_count = 0;
    if (usbd_desc_cfg) {
        return;
    }

    int hid_report_len;
    if (GetDescHIDReport(&hid_report_len)) {
        uint8_t hid_desc[TUD_HID_DESC_LEN] = {
            // Interface number, string index, protocol, report descriptor len, EP In & Out address, size & polling interval
            TUD_HID_DESCRIPTOR(1 /* placeholder*/, 0, HID_ITF_PROTOCOL_NONE, hid_report_len, __hid_endpoint = USB.registerEndpointIn(), CFG_TUD_HID_EP_BUFSIZE, (uint8_t)usb_hid_poll_interval)
        };
        __hid_interface = USB.registerInterface(1, hid_desc, sizeof(hid_desc), 10, 0);
    }

#ifdef ENABLE_PICOTOOL_USB
    uint8_t picotool_desc[] = { TUD_RPI_RESET_DESCRIPTOR(1, USB.registerString("Reset")) };
    USB.registerInterface(1, picotool_desc, sizeof(picotool_desc), 100, 0);
#endif

    usbd_desc_cfg_len = TUD_CONFIG_DESC_LEN; // Always have a config descriptor
    Entry *h = _interfaces;
    while (h) {
        usbd_desc_cfg_len += h->len;
        interface_count += h->interfaces;
        h = h->next;
    }

    uint8_t tud_cfg_desc[TUD_CONFIG_DESC_LEN] = {
        // Config number, interface count, string index, total length, attribute, power in mA
        TUD_CONFIG_DESCRIPTOR(1, interface_count, USB.registerString(""), usbd_desc_cfg_len, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, USBD_MAX_POWER_MA)
    };

    // Allocate the "real" HID report descriptor
    usbd_desc_cfg = (uint8_t *)malloc(usbd_desc_cfg_len);
    assert(usbd_desc_cfg);
    bzero(usbd_desc_cfg, usbd_desc_cfg_len);

    // Now copy the descriptors
    h = _interfaces;
    uint8_t *p = usbd_desc_cfg;
    memcpy(p, tud_cfg_desc, sizeof(tud_cfg_desc));
    p += sizeof(tud_cfg_desc);
    int id = 0;
    while (h) {
        memcpy(p, h->descriptor, h->len);
        p[2] = id; // Set the interface
        p += h->len;
        id += h->interfaces;
        h = h->next;
    }
}


const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;
#define DESC_STR_MAX (32)
    static uint16_t desc_str[DESC_STR_MAX];
    uint8_t len;
    if (index == 0) {
        desc_str[1] = 0x0409; // supported language is English
        len = 1;
    } else {
        if (index >= usbd_desc_str_cnt) {
            return nullptr;
        }
        const char *str = usbd_desc_str[index];
        for (len = 0; len < DESC_STR_MAX - 1 && str[len]; ++len) {
            desc_str[1 + len] = str[len];
        }
    }

    // first byte is length (including header), second byte is string type
    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * len + 2);

    return desc_str;
}

#ifdef __FREERTOS
void __freertos_usb_task(void *param) {
    (void) param;

    Serial.begin(115200);

    USB.initted = true;

    while (true) {
        BaseType_t ss = xTaskGetSchedulerState();
        if (ss != taskSCHEDULER_SUSPENDED) {
            auto m = __get_freertos_mutex_for_ptr(&USB.mutex);
            if (xSemaphoreTake(m, 0)) {
                tud_task();
                xSemaphoreGive(m);
            }
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
#else
static int __usb_task_irq;
static void usb_irq() {
    // if the mutex is already owned, then we are in user code
    // in this file which will do a tud_task itself, so we'll just do nothing
    // until the next tick; we won't starve
    if (mutex_try_enter(&USB.mutex, nullptr)) {
        tud_task();
        mutex_exit(&USB.mutex);
    }
}

static int64_t timer_task(__unused alarm_id_t id, __unused void *user_data) {
    irq_set_pending(__usb_task_irq);
    return USB_TASK_INTERVAL;
}
#endif

void RP2040USB::disconnect() {
    bool wasConnected = tud_connected();
#ifdef __FREERTOS
    auto m = __get_freertos_mutex_for_ptr(&USB.mutex);
    xSemaphoreTake(m, portMAX_DELAY);
    tud_disconnect();
    if (wasConnected) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    xSemaphoreGive(m);
#else
    mutex_enter_blocking(&USB.mutex);
    tud_disconnect();
    if (wasConnected) {
        sleep_ms(500);
    }
    mutex_exit(&USB.mutex);
#endif
    // Ensure when we reconnect we make the new descriptor
    free(usbd_desc_cfg);
    usbd_desc_cfg = nullptr;
    usbd_desc_cfg_len = 0;
    if (__hid_report) {
        unregisterInterface(__hid_interface);
        unregisterEndpointIn(__hid_endpoint);
    }
    free(__hid_report);
    __hid_report = nullptr;
    __hid_report_len = 0;
}

void RP2040USB::connect()  {
    __SetupDescHIDReport();
    __SetupUSBDescriptor();

#ifdef __FREERTOS
    auto m = __get_freertos_mutex_for_ptr(&USB.mutex);
    xSemaphoreTake(m, portMAX_DELAY);
    tud_connect();
    xSemaphoreGive(m);
#else
    mutex_enter_blocking(&USB.mutex);
    tud_connect();
    mutex_exit(&USB.mutex);
#endif
}






void RP2040USB::begin() {
    if (tusb_inited()) {
        // Already called
        return;
    }

    __SetupDescHIDReport();
    __SetupUSBDescriptor();

    mutex_init(&mutex);

    tusb_init();

#ifdef __FREERTOS
    // Make high prio and locked to core 0
    TaskHandle_t usbTask;
    xTaskCreate(__freertos_usb_task, "USB", 256, 0, configMAX_PRIORITIES - 2, &usbTask);
    vTaskCoreAffinitySet(usbTask, 1 << 0);
#else
    __usb_task_irq = user_irq_claim_unused(true);
    irq_set_exclusive_handler(__usb_task_irq, usb_irq);
    irq_set_enabled(__usb_task_irq, true);
    add_alarm_in_us(USB_TASK_INTERVAL, timer_task, nullptr, true);
#endif
}


bool RP2040USB::HIDReady() {
    uint32_t start = millis();
    const uint32_t timeout = 500;

    while (((millis() - start) < timeout) && tud_ready() && !tud_hid_ready()) {
        tud_task();
        delayMicroseconds(1);
    }
    return tud_hid_ready();
}


// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
extern "C" uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) __attribute__((weak));
extern "C" uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    // TODO not implemented
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
extern "C" void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) __attribute__((weak));
extern "C" void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    // TODO set LED based on CAPLOCK, NUMLOCK etc...
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}

extern "C" int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) __attribute__((weak));
extern "C" int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    (void) lun;
    (void) lba;
    (void) offset;
    (void) buffer;
    (void) bufsize;
    return -1;
}

extern "C" bool tud_msc_test_unit_ready_cb(uint8_t lun) __attribute__((weak));
extern "C" bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    (void) lun;
    return false;
}

extern "C" int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) __attribute__((weak));
extern "C" int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    (void) lun;
    (void) lba;
    (void) offset;
    (void) buffer;
    (void) bufsize;
    return -1;
}

extern "C" int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize) __attribute__((weak));
extern "C" int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize) {
    (void) lun;
    (void) scsi_cmd;
    (void) buffer;
    (void) bufsize;
    return 0;
}

extern "C" void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size) __attribute__((weak));
extern "C" void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size) {
    (void) lun;
    *block_count = 0;
    *block_size = 0;
}

extern "C" void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) __attribute__((weak));
extern "C" void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
    (void) lun;
    vendor_id[0] = 0;
    product_id[0] = 0;
    product_rev[0] = 0;
}



#ifdef ENABLE_PICOTOOL_USB

static void resetd_init() {
}

static void resetd_reset(uint8_t rhport) {
    (void) rhport;
    _picotool_itf_num = 0;
}

static uint16_t resetd_open(uint8_t rhport,
                            tusb_desc_interface_t const *itf_desc, uint16_t max_len) {
    (void) rhport;
    TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass &&
              RESET_INTERFACE_SUBCLASS == itf_desc->bInterfaceSubClass &&
              RESET_INTERFACE_PROTOCOL == itf_desc->bInterfaceProtocol, 0);

    uint16_t const drv_len = sizeof(tusb_desc_interface_t);
    TU_VERIFY(max_len >= drv_len, 0);

    _picotool_itf_num = itf_desc->bInterfaceNumber;
    return drv_len;
}

// Support for parameterized reset via vendor interface control request
static bool resetd_control_xfer_cb(uint8_t rhport, uint8_t stage,
                                   tusb_control_request_t const *request) {
    (void) rhport;
    // nothing to do with DATA & ACK stage
    if (stage != CONTROL_STAGE_SETUP) {
        return true;
    }

    if (request->wIndex == _picotool_itf_num) {
        if (request->bRequest == RESET_REQUEST_BOOTSEL) {
            reset_usb_boot(0, (request->wValue & 0x7f));
            // does not return, otherwise we'd return true
        }

        if (request->bRequest == RESET_REQUEST_FLASH) {
            watchdog_reboot(0, 0, 100);
            return true;
        }

    }
    return false;
}

static bool resetd_xfer_cb(uint8_t rhport, uint8_t ep_addr,
                           xfer_result_t result, uint32_t xferred_bytes) {
    (void) rhport;
    (void) ep_addr;
    (void) result;
    (void) xferred_bytes;
    return true;
}

static usbd_class_driver_t const _resetd_driver = {
#if CFG_TUSB_DEBUG >= 2
    .name = "RESET",
#endif
    .init             = resetd_init,
    .reset            = resetd_reset,
    .open             = resetd_open,
    .control_xfer_cb  = resetd_control_xfer_cb,
    .xfer_cb          = resetd_xfer_cb,
    .sof              = NULL
};

// Implement callback to add our custom driver
usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;
    return &_resetd_driver;
}

#elif defined NO_USB

#warning "NO_USB selected. No output to Serial will occur!"

#include <Arduino.h>

void SerialUSB::begin(unsigned long baud) {
}

void SerialUSB::end() {

}

int SerialUSB::peek() {
    return 0;
}

int SerialUSB::read() {
    return -1;
}

int SerialUSB::available() {
    return 0;
}

int SerialUSB::availableForWrite() {
    return 0;
}

void SerialUSB::flush() {

}

size_t SerialUSB::write(uint8_t c) {
    (void) c;
    return 0;
}

size_t SerialUSB::write(const uint8_t *buf, size_t length) {
    (void) buf;
    (void) length;
    return 0;
}

SerialUSB::operator bool() {
    return false;
}

SerialUSB Serial;

#endif


#endif
