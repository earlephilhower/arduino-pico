/*
    Serial-Over-USB for the Raspberry Pi Pico RP2040
    Implements an ACM which will reboot into UF2 mode on a 1200bps DTR toggle.
    Much of this was modified from the Raspberry Pi Pico SDK stdio_usb.c file.

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

#include "tusb.h"
#include "pico/time.h"
#include "pico/binary_info.h"
#include "pico/bootrom.h"
#include "hardware/irq.h"
#include "pico/mutex.h"
#include "hardware/watchdog.h"
#include "pico/unique_id.h"

#ifndef DISABLE_USB_SERIAL
// Ensure we are installed in the USB chain
void __USBInstallSerial() { /* noop */ }
#endif

// SerialEvent functions are weak, so when the user doesn't define them,
// the linker just sets their address to 0 (which is checked below).
// The Serialx_available is just a wrapper around Serialx.available(),
// but we can refer to it weakly so we don't pull in the entire
// HardwareSerial instance if the user doesn't also refer to it.
extern void serialEvent() __attribute__((weak));


extern mutex_t __usb_mutex;

void SerialUSB::begin(unsigned long baud) {
    (void) baud; //ignored

    if (_running) {
        return;
    }

    _running = true;
}

void SerialUSB::end() {
    // TODO
}

int SerialUSB::peek() {
    CoreMutex m(&__usb_mutex);
    if (!_running || !m) {
        return 0;
    }

    uint8_t c;
    return tud_cdc_peek(&c) ? (int) c : -1;
}

int SerialUSB::read() {
    CoreMutex m(&__usb_mutex);
    if (!_running || !m) {
        return -1;
    }

    if (tud_cdc_connected() && tud_cdc_available()) {
        return tud_cdc_read_char();
    }
    return -1;
}

int SerialUSB::available() {
    CoreMutex m(&__usb_mutex);
    if (!_running || !m) {
        return 0;
    }

    return tud_cdc_available();
}

int SerialUSB::availableForWrite() {
    CoreMutex m(&__usb_mutex);
    if (!_running || !m) {
        return 0;
    }

    return tud_cdc_write_available();
}

void SerialUSB::flush() {
    CoreMutex m(&__usb_mutex);
    if (!_running || !m) {
        return;
    }

    tud_cdc_write_flush();
}

size_t SerialUSB::write(uint8_t c) {
    return write(&c, 1);
}

size_t SerialUSB::write(const uint8_t *buf, size_t length) {
    CoreMutex m(&__usb_mutex);
    if (!_running || !m) {
        return 0;
    }

    static uint64_t last_avail_time;
    int i = 0;
    if (tud_cdc_connected()) {
        for (size_t i = 0; i < length;) {
            int n = length - i;
            int avail = tud_cdc_write_available();
            if (n > avail) {
                n = avail;
            }
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
    CoreMutex m(&__usb_mutex);
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
        reset_usb_boot(0, 0);
        while (1); // WDT will fire here
    }
}

extern "C" void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    (void) itf;
    _dtr = dtr ? true : false;
    _rts = rts ? true : false;
    CheckSerialReset();
}

extern "C" void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding) {
    (void) itf;
    _bps = p_line_coding->bit_rate;
    CheckSerialReset();
}

SerialUSB Serial;

void arduino::serialEventRun(void) {
    if (serialEvent && Serial.available()) {
        serialEvent();
    }
}

#endif
