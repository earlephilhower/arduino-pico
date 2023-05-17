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
#include <class/audio/audio.h>
#include <pico/time.h>
#include <hardware/irq.h>
#include <pico/mutex.h>
#include <pico/unique_id.h>
#include <pico/usb_reset_interface.h>
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include "sdkoverride/tusb_absmouse.h"
#include <device/usbd_pvt.h>

// Big, global USB mutex, shared with all USB devices to make sure we don't
// have multiple cores updating the TUSB state in parallel
mutex_t __usb_mutex;

// USB processing will be a periodic timer task
#define USB_TASK_INTERVAL 1000
static int __usb_task_irq;

#ifndef USBD_VID
#define USBD_VID (0x2E8A) // Raspberry Pi
#endif

#ifndef USBD_PID
#define USBD_PID (0x000a) // Raspberry Pi Pico SDK CDC
#endif

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

#define USBD_ITF_CDC (0) // needs 2 interfaces
#define USBD_ITF_MAX (2)

#define USBD_CDC_EP_CMD (0x81)
#define USBD_CDC_EP_OUT (0x02)
#define USBD_CDC_EP_IN (0x82)
#define USBD_CDC_CMD_MAX_SIZE (8)
#define USBD_CDC_IN_OUT_MAX_SIZE (64)

#define USBD_STR_0 (0x00)
#define USBD_STR_MANUF (0x01)
#define USBD_STR_PRODUCT (0x02)
#define USBD_STR_SERIAL (0x03)
#define USBD_STR_CDC (0x04)
#define USBD_STR_RPI_RESET (0x05)

#define EPNUM_HID   0x83

#define USBD_MSC_EPOUT 0x03
#define USBD_MSC_EPIN 0x84
#define USBD_MSC_EPSIZE 64

#define TUD_RPI_RESET_DESCRIPTOR(_itfnum, _stridx) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 0, TUSB_CLASS_VENDOR_SPECIFIC, RESET_INTERFACE_SUBCLASS, RESET_INTERFACE_PROTOCOL, _stridx,


const uint8_t *tud_descriptor_device_cb(void) {
    static tusb_desc_device_t usbd_desc_device = {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0200,
        .bDeviceClass = TUSB_CLASS_CDC,
        .bDeviceSubClass = MISC_SUBCLASS_COMMON,
        .bDeviceProtocol = MISC_PROTOCOL_IAD,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
        .idVendor = USBD_VID,
        .idProduct = USBD_PID,
        .bcdDevice = 0x0100,
        .iManufacturer = USBD_STR_MANUF,
        .iProduct = USBD_STR_PRODUCT,
        .iSerialNumber = USBD_STR_SERIAL,
        .bNumConfigurations = 1
    };
    if (__USBInstallSerial && !__USBInstallKeyboard && !__USBInstallMouse && !__USBInstallAbsoluteMouse && !__USBInstallJoystick && !__USBInstallMassStorage) {
        // Can use as-is, this is the default USB case
        return (const uint8_t *)&usbd_desc_device;
    }
    // Need a multi-endpoint config which will require changing the PID to help Windows not barf
    if (__USBInstallKeyboard) {
        usbd_desc_device.idProduct |= 0x8000;
    }
    if (__USBInstallMouse || __USBInstallAbsoluteMouse) {
        usbd_desc_device.idProduct |= 0x4000;
    }
    if (__USBInstallJoystick) {
        usbd_desc_device.idProduct |= 0x0100;
    }
    if (__USBInstallMassStorage) {
        usbd_desc_device.idProduct ^= 0x2000;
    }
    // Set the device class to 0 to indicate multiple device classes
    usbd_desc_device.bDeviceClass = 0;
    usbd_desc_device.bDeviceSubClass = 0;
    usbd_desc_device.bDeviceProtocol = 0;
    return (const uint8_t *)&usbd_desc_device;
}

int __USBGetKeyboardReportID() {
    return 1;
}

int __USBGetMouseReportID() {
    return __USBInstallKeyboard ? 3 : 1;
}

int __USBGetJoystickReportID() {
    int i = 1;
    if (__USBInstallKeyboard) {
        i += 2;
    }
    if (__USBInstallMouse || __USBInstallAbsoluteMouse) {
        i++;
    }
    return i;
}

static int      __hid_report_len = 0;
static uint8_t *__hid_report     = nullptr;

static uint8_t *GetDescHIDReport(int *len) {
    if (len) {
        *len = __hid_report_len;
    }
    return __hid_report;
}

void __SetupDescHIDReport() {
    //allocate memory for the HID report descriptors. We don't use them, but need the size here.
    uint8_t desc_hid_report_mouse[] = { TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(1)) };
    uint8_t desc_hid_report_absmouse[] = { TUD_HID_REPORT_DESC_ABSMOUSE(HID_REPORT_ID(1)) };
    uint8_t desc_hid_report_joystick[] = { TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(1)) };
    uint8_t desc_hid_report_keyboard[] = { TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)), TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(2)) };
    int size = 0;

    //accumulate the size of all used HID report descriptors
    if (__USBInstallKeyboard) {
        size += sizeof(desc_hid_report_keyboard);
    }
    if (__USBInstallMouse) {
        size += sizeof(desc_hid_report_mouse);
    } else if (__USBInstallAbsoluteMouse) {
        size += sizeof(desc_hid_report_absmouse);
    }
    if (__USBInstallJoystick) {
        size += sizeof(desc_hid_report_joystick);
    }

    //no HID used at all
    if (size == 0) {
        __hid_report = nullptr;
        __hid_report_len = 0;
        return;
    }

    //allocate the "real" HID report descriptor
    __hid_report = (uint8_t *)malloc(size);
    if (__hid_report) {
        __hid_report_len = size;

        //now copy the descriptors

        //1.) keyboard descriptor, if requested
        if (__USBInstallKeyboard) {
            memcpy(__hid_report, desc_hid_report_keyboard, sizeof(desc_hid_report_keyboard));
        }

        //2.) mouse descriptor, if necessary. Additional offset & new array is necessary if there is a keyboard.
        if (__USBInstallMouse) {
            //determine if we need an offset (USB keyboard is installed)
            if (__USBInstallKeyboard) {
                uint8_t desc_local[] = { TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(3)) };
                memcpy(__hid_report + sizeof(desc_hid_report_keyboard), desc_local, sizeof(desc_local));
            } else {
                memcpy(__hid_report, desc_hid_report_mouse, sizeof(desc_hid_report_mouse));
            }
        } else if (__USBInstallAbsoluteMouse) {
            //determine if we need an offset (USB keyboard is installed)
            if (__USBInstallKeyboard) {
                uint8_t desc_local[] = { TUD_HID_REPORT_DESC_ABSMOUSE(HID_REPORT_ID(3)) };
                memcpy(__hid_report + sizeof(desc_hid_report_keyboard), desc_local, sizeof(desc_local));
            } else {
                memcpy(__hid_report, desc_hid_report_absmouse, sizeof(desc_hid_report_absmouse));
            }
        }

        //3.) joystick descriptor. 2 additional checks are necessary for mouse and/or keyboard
        if (__USBInstallJoystick) {
            uint8_t reportid = 1;
            int offset = 0;
            if (__USBInstallKeyboard) {
                reportid += 2;
                offset += sizeof(desc_hid_report_keyboard);
            }
            if (__USBInstallMouse) {
                reportid++;
                offset += sizeof(desc_hid_report_mouse);
            } else if (__USBInstallAbsoluteMouse) {
                reportid++;
                offset += sizeof(desc_hid_report_absmouse);
            }
            uint8_t desc_local[] = { TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(reportid)) };
            memcpy(__hid_report + offset, desc_local, sizeof(desc_local));
        }
    }
}

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return GetDescHIDReport(nullptr);
}

static uint8_t *usbd_desc_cfg = nullptr;
const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return usbd_desc_cfg;
}

void __SetupUSBDescriptor() {
    if (!usbd_desc_cfg) {
        bool hasHID = __USBInstallKeyboard || __USBInstallMouse || __USBInstallAbsoluteMouse || __USBInstallJoystick;

        uint8_t interface_count = (__USBInstallSerial ? 2 : 0) + (hasHID ? 1 : 0) + (__USBInstallMassStorage ? 1 : 0);

        uint8_t cdc_desc[TUD_CDC_DESC_LEN] = {
            // Interface number, string index, protocol, report descriptor len, EP In & Out address, size & polling interval
            TUD_CDC_DESCRIPTOR(USBD_ITF_CDC, USBD_STR_CDC, USBD_CDC_EP_CMD, USBD_CDC_CMD_MAX_SIZE, USBD_CDC_EP_OUT, USBD_CDC_EP_IN, USBD_CDC_IN_OUT_MAX_SIZE)
        };

        int hid_report_len;
        GetDescHIDReport(&hid_report_len);
        uint8_t hid_itf = __USBInstallSerial ? 2 : 0;
        uint8_t hid_desc[TUD_HID_DESC_LEN] = {
            // Interface number, string index, protocol, report descriptor len, EP In & Out address, size & polling interval
            TUD_HID_DESCRIPTOR(hid_itf, 0, HID_ITF_PROTOCOL_NONE, hid_report_len, EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 10)
        };

        uint8_t msd_itf = interface_count - 1;
        uint8_t msd_desc[TUD_MSC_DESC_LEN] = {
            TUD_MSC_DESCRIPTOR(msd_itf, 0, USBD_MSC_EPOUT, USBD_MSC_EPIN, USBD_MSC_EPSIZE)
        };

        int usbd_desc_len = TUD_CONFIG_DESC_LEN + (__USBInstallSerial ? sizeof(cdc_desc) : 0) + (hasHID ? sizeof(hid_desc) : 0) + (__USBInstallMassStorage ? sizeof(msd_desc) : 0);

#ifdef ENABLE_PICOTOOL_USB
        uint8_t picotool_itf = interface_count++;
        uint8_t picotool_desc[] = {
            TUD_RPI_RESET_DESCRIPTOR(picotool_itf, USBD_STR_RPI_RESET)
        };
        usbd_desc_len += sizeof(picotool_desc);
#endif

        uint8_t tud_cfg_desc[TUD_CONFIG_DESC_LEN] = {
            // Config number, interface count, string index, total length, attribute, power in mA
            TUD_CONFIG_DESCRIPTOR(1, interface_count, USBD_STR_0, usbd_desc_len, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, USBD_MAX_POWER_MA)
        };

        // Combine to one descriptor
        usbd_desc_cfg = (uint8_t *)malloc(usbd_desc_len);
        if (usbd_desc_cfg) {
            bzero(usbd_desc_cfg, usbd_desc_len);
            uint8_t *ptr = usbd_desc_cfg;
            memcpy(ptr, tud_cfg_desc, sizeof(tud_cfg_desc));
            ptr += sizeof(tud_cfg_desc);
            if (__USBInstallSerial) {
                memcpy(ptr, cdc_desc, sizeof(cdc_desc));
                ptr += sizeof(cdc_desc);
            }
            if (hasHID) {
                memcpy(ptr, hid_desc, sizeof(hid_desc));
                ptr += sizeof(hid_desc);
            }
            if (__USBInstallMassStorage) {
                memcpy(ptr, msd_desc, sizeof(msd_desc));
                ptr += sizeof(msd_desc);
            }
#ifdef ENABLE_PICOTOOL_USB
            memcpy(ptr, picotool_desc, sizeof(picotool_desc));
            ptr += sizeof(picotool_desc);
#endif
        }
    }
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;
#define DESC_STR_MAX (32)
    static uint16_t desc_str[DESC_STR_MAX];

    static char idString[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

    static const char *const usbd_desc_str[] = {
        [USBD_STR_0] = "",
        [USBD_STR_MANUF] = USB_MANUFACTURER,
        [USBD_STR_PRODUCT] = USB_PRODUCT,
        [USBD_STR_SERIAL] = idString,
        [USBD_STR_CDC] = "Board CDC",
#ifdef ENABLE_PICOTOOL_USB
        [USBD_STR_RPI_RESET] = "Reset",
#endif
    };

    if (!idString[0]) {
        pico_get_unique_board_id_string(idString, sizeof(idString));
    }

    uint8_t len;
    if (index == 0) {
        desc_str[1] = 0x0409; // supported language is English
        len = 1;
    } else {
        if (index >= sizeof(usbd_desc_str) / sizeof(usbd_desc_str[0])) {
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


static void usb_irq() {
    // if the mutex is already owned, then we are in user code
    // in this file which will do a tud_task itself, so we'll just do nothing
    // until the next tick; we won't starve
    if (mutex_try_enter(&__usb_mutex, nullptr)) {
        tud_task();
        mutex_exit(&__usb_mutex);
    }
}

static int64_t timer_task(__unused alarm_id_t id, __unused void *user_data) {
    irq_set_pending(__usb_task_irq);
    return USB_TASK_INTERVAL;
}

void __USBStart() __attribute__((weak));

void __USBStart() {
    if (tusb_inited()) {
        // Already called
        return;
    }

    __SetupDescHIDReport();
    __SetupUSBDescriptor();

    mutex_init(&__usb_mutex);

    tusb_init();

    __usb_task_irq = user_irq_claim_unused(true);
    irq_set_exclusive_handler(__usb_task_irq, usb_irq);
    irq_set_enabled(__usb_task_irq, true);

    add_alarm_in_us(USB_TASK_INTERVAL, timer_task, nullptr, true);
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

static uint8_t _picotool_itf_num;

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

// will ensure backward compatibility with existing code when using pico-debug

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
