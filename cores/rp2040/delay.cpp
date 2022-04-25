/*
    delay() for the Raspberry Pi Pico RP2040

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

#include <pico.h>
#include <pico/time.h>

#ifdef USE_TINYUSB
#include "Adafruit_TinyUSB_API.h"
#endif

extern "C" void delay(unsigned long ms) __attribute__((weak));
extern "C" void yield() __attribute__((weak));

extern "C"
{

    void delay(unsigned long ms) {
        if (!ms) {
            return;
        }

        sleep_ms(ms);
    }

    void delayMicroseconds(unsigned int usec) {
        if (!usec) {
            return;
        }
        sleep_us(usec);
    }

    void yield() {
#ifdef USE_TINYUSB
        TinyUSB_Device_Task();
        TinyUSB_Device_FlushCDC();
#endif
    }


    uint32_t millis() {
        return to_ms_since_boot(get_absolute_time());
    }

    uint32_t micros() {
        return to_us_since_boot(get_absolute_time());
    }

} // extern C
