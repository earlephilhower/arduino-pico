/*
    Main handler for the Raspberry Pi Pico RP2040

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

#include <Arduino.h>
#include "RP2040USB.h"
#include <pico/stdlib.h>
#include <pico/multicore.h>

RP2040 rp2040;
extern "C" {
    volatile bool __otherCoreIdled = false;
};

mutex_t _pioMutex;

extern void setup();
extern void loop();

// FreeRTOS potential includes
extern void initFreeRTOS() __attribute__((weak));
extern void startFreeRTOS() __attribute__((weak));
bool __isFreeRTOS;

// Weak empty variant initialization. May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

// Optional 2nd core setup and loop
extern void setup1() __attribute__((weak));
extern void loop1() __attribute__((weak));
extern "C" void main1() {
    rp2040.fifo.registerCore();
    if (setup1) {
        setup1();
    }
    while (true) {
        if (loop1) {
            loop1();
        }
    }
}

extern void __loop() {
#ifdef USE_TINYUSB
    yield();
#endif

    if (arduino::serialEventRun) {
        arduino::serialEventRun();
    }
    if (arduino::serialEvent1Run) {
        arduino::serialEvent1Run();
    }
    if (arduino::serialEvent2Run) {
        arduino::serialEvent2Run();
    }
}

extern "C" int main() {
#if F_CPU != 125000000
    set_sys_clock_khz(F_CPU / 1000, true);
#endif

    // Let rest of core know if we're using FreeRTOS
    __isFreeRTOS = initFreeRTOS ? true : false;

    mutex_init(&_pioMutex);

    rp2040.begin();

    initVariant();

    if (__isFreeRTOS) {
        initFreeRTOS();
    }

#ifndef NO_USB
#ifdef USE_TINYUSB
    TinyUSB_Device_Init(0);

#else
    __USBStart();

#ifndef DISABLE_USB_SERIAL

    if (!__isFreeRTOS) {
        // Enable serial port for reset/upload always
        Serial.begin(115200);
    }
#endif
#endif
#endif

#if defined DEBUG_RP2040_PORT
    if (!__isFreeRTOS) {
        DEBUG_RP2040_PORT.begin(115200);
    }
#endif

#ifndef NO_USB
    if (!__isFreeRTOS) {
        if (setup1 || loop1) {
            rp2040.fifo.begin(2);
            multicore_launch_core1(main1);
        } else {
            rp2040.fifo.begin(1);
        }
        rp2040.fifo.registerCore();
    }
#endif

    if (!__isFreeRTOS) {
        setup();
        while (true) {
            loop();
            __loop();
        }
    } else {
        rp2040.fifo.begin(2);
        startFreeRTOS();
    }
    return 0;
}

extern "C" unsigned long ulMainGetRunTimeCounterValue() {
    return rp2040.getCycleCount64();
}
