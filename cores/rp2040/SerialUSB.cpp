/*
 * Serial-Over-USB for the Raspberry Pi Pico RP2040
 * Implements an ACM which will reboot into UF2 mode on a 1200bps DTR toggle.
 * Much of this was modified from the Raspberry Pi Pico SDK stdio_usb.c file.
 *
 * Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <Arduino.h>
#include "CoreMutex.h"

#include "tusb.h"
#include "pico/time.h"
#include "pico/binary_info.h"
#include "pico/bootrom.h"
#include "hardware/irq.h"
#include "pico/mutex.h"
#include "hardware/watchdog.h"
#include "pico/unique_id.h"

// SerialEvent functions are weak, so when the user doesn't define them,
// the linker just sets their address to 0 (which is checked below).
// The Serialx_available is just a wrapper around Serialx.available(),
// but we can refer to it weakly so we don't pull in the entire
// HardwareSerial instance if the user doesn't also refer to it.
extern void serialEvent() __attribute__((weak));

#define PICO_STDIO_USB_TASK_INTERVAL_US 1000
#define PICO_STDIO_USB_LOW_PRIORITY_IRQ 31

#define USBD_VID (0x2E8A) // Raspberry Pi

#ifdef SERIALUSB_PID
    #define USBD_PID (SERIALUSB_PID)
#else
    #define USBD_PID (0x000a) // Raspberry Pi Pico SDK CDC
#endif

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)
#define USBD_MAX_POWER_MA (250)

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

// Note: descriptors returned from callbacks must exist long enough for transfer to complete

static const tusb_desc_device_t usbd_desc_device = {
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
    .bNumConfigurations = 1,
};

static const uint8_t usbd_desc_cfg[USBD_DESC_LEN] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_0, USBD_DESC_LEN,
        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, USBD_MAX_POWER_MA),

    TUD_CDC_DESCRIPTOR(USBD_ITF_CDC, USBD_STR_CDC, USBD_CDC_EP_CMD,
        USBD_CDC_CMD_MAX_SIZE, USBD_CDC_EP_OUT, USBD_CDC_EP_IN, USBD_CDC_IN_OUT_MAX_SIZE),
};

static char _idString[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

static const char *const usbd_desc_str[] = {
    [USBD_STR_0] = "",
    [USBD_STR_MANUF] = "Raspberry Pi",
    [USBD_STR_PRODUCT] = "PicoArduino",
    [USBD_STR_SERIAL] = _idString,
    [USBD_STR_CDC] = "Board CDC",
};

const uint8_t *tud_descriptor_device_cb(void) {
    return (const uint8_t *)&usbd_desc_device;
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return usbd_desc_cfg;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    #define DESC_STR_MAX (20)
    static uint16_t desc_str[DESC_STR_MAX];

    uint8_t len;
    if (index == 0) {
        desc_str[1] = 0x0409; // supported language is English
        len = 1;
    } else {
        if (index >= sizeof(usbd_desc_str) / sizeof(usbd_desc_str[0])) {
            return NULL;
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

static mutex_t usb_mutex;

static void low_priority_worker_irq() {
    // if the mutex is already owned, then we are in user code
    // in this file which will do a tud_task itself, so we'll just do nothing
    // until the next tick; we won't starve
    if (mutex_try_enter(&usb_mutex, NULL)) {
        tud_task();
        mutex_exit(&usb_mutex);
    }
}

static int64_t timer_task(__unused alarm_id_t id, __unused void *user_data) {
    irq_set_pending(PICO_STDIO_USB_LOW_PRIORITY_IRQ);
    return PICO_STDIO_USB_TASK_INTERVAL_US;
}

void SerialUSB::begin(unsigned long baud) {
    (void) baud; //ignored

    if (_running) {
        return;
    }

    // Get ID string into human readable serial number
    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);
    _idString[0] = 0;
    for (auto i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i++) {
        char hx[3];
        sprintf(hx, "%02X", id.id[i]);
        strcat(_idString, hx);
    }

    tusb_init();

    irq_set_exclusive_handler(PICO_STDIO_USB_LOW_PRIORITY_IRQ, low_priority_worker_irq);
    irq_set_enabled(PICO_STDIO_USB_LOW_PRIORITY_IRQ, true);

    mutex_init(&usb_mutex);
    add_alarm_in_us(PICO_STDIO_USB_TASK_INTERVAL_US, timer_task, NULL, true);

    _running = true;
}

void SerialUSB::end() {
    // TODO
}

int SerialUSB::peek() {
    CoreMutex m(&usb_mutex);
    if (!_running || !m) {
        return 0;
    }

    uint8_t c;
    return tud_cdc_peek(0, &c) ? (int) c : -1;
}

int SerialUSB::read() {
    CoreMutex m(&usb_mutex);
    if (!_running || !m) {
        return -1;
    }

    if (tud_cdc_connected() && tud_cdc_available()) {
        return tud_cdc_read_char();
    }
    return -1;
}

int SerialUSB::available() {
    CoreMutex m(&usb_mutex);
    if (!_running || !m) {
        return 0;
    }

    return tud_cdc_available();
}

int SerialUSB::availableForWrite() {
    CoreMutex m(&usb_mutex);
    if (!_running || !m) {
        return 0;
    }

    return tud_cdc_write_available();
}

void SerialUSB::flush() {
    CoreMutex m(&usb_mutex);
    if (!_running || !m) {
        return;
    }

    tud_cdc_write_flush();
}

size_t SerialUSB::write(uint8_t c) {
    return write(&c, 1);
}

size_t SerialUSB::write(const uint8_t *buf, size_t length) {
    CoreMutex m(&usb_mutex);
    if (!_running || !m) {
        return 0;
    }

    static uint64_t last_avail_time;
    int i = 0;
    if (tud_cdc_connected()) {
        for (int i = 0; i < length;) {
            int n = length - i;
            int avail = tud_cdc_write_available();
            if (n > avail) n = avail;
            if (n) {
                int n2 = tud_cdc_write(buf + i, n);
                tud_task();
                tud_cdc_write_flush();
                i += n2;
                last_avail_time = time_us_64();
            } else {
                tud_task();
                tud_cdc_write_flush();
                if (!tud_cdc_connected() ||
                    (!tud_cdc_write_available() && time_us_64() > last_avail_time + 1000000 /* 1 second */)) {
                    break;
                }
            }
        }
    } else {
        // reset our timeout
        last_avail_time = 0;
    }
    return i;
}

SerialUSB::operator bool() {
    CoreMutex m(&usb_mutex);
    if (!_running || !m) {
        return false;
    }

    tud_task();
    return tud_cdc_connected();
}


static bool _dtr = false;
static bool _rts = false;
static int _bps = 115200;
static void CheckSerialReset() {
    if ((_bps == 1200) && (!_dtr)) {
        reset_usb_boot(0,0);
	while (1); // WDT will fire here
    }
}

extern "C" void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    _dtr = dtr ? true : false;
    _rts = rts ? true : false;
    CheckSerialReset();
}

extern "C" void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding) {
    _bps = p_line_coding->bit_rate;
    CheckSerialReset();
}

SerialUSB Serial;

void arduino::serialEventRun(void)
{
    if (serialEvent && Serial.available()) {
      serialEvent();
    }
}
