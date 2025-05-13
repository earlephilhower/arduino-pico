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
#include <hardware/vreg.h>
#include <reent.h>
#ifdef RP2350_PSRAM_CS
#include "psram.h"
#endif

RP2040 rp2040;
extern "C" {
    volatile bool __otherCoreIdled = false;
    uint32_t* core1_separate_stack_address = nullptr;
};

extern void setup();
extern void loop();

// FreeRTOS potential includes
extern void initFreeRTOS() __attribute__((weak));
extern void startFreeRTOS() __attribute__((weak));
bool __isFreeRTOS;
volatile bool __freeRTOSinitted;

extern void __EnableBluetoothDebug(Print &);

// Weak empty variant initialization. May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

// Optional 2nd core setup and loop
bool core1_separate_stack __attribute__((weak)) = false;
bool core1_disable_systick __attribute__((weak)) = false;
extern void setup1() __attribute__((weak));
extern void loop1() __attribute__((weak));
extern "C" void main1() {
    if (!core1_disable_systick) {
        // Don't install the SYSTICK exception handler. rp2040.getCycleCount will not work properly on core1
        rp2040.begin(1);
    }
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
static struct _reent *_impure_ptr1 = nullptr;

extern "C" int main() {
#if (defined(PICO_RP2040) && (F_CPU != 125000000)) || (defined(PICO_RP2350) && (F_CPU != 150000000))

#if defined(PICO_RP2040)
    // From runtime_init_clocks() to bump up RP2040 V for 200Mhz+ operation
    if ((F_CPU > 133000000) && (vreg_get_voltage() < VREG_VOLTAGE_1_15)) {
        vreg_set_voltage(VREG_VOLTAGE_1_15);
        // wait for voltage to settle; must use CPU cycles as TIMER is not yet clocked correctly
        busy_wait_at_least_cycles((uint32_t)((SYS_CLK_VREG_VOLTAGE_AUTO_ADJUST_DELAY_US * (uint64_t)XOSC_HZ) / 1000000));
    }
#endif

#if defined(RP2350_PSRAM_CS) && (F_CPU > 150000000)
    // Need to increase the qmi divider before upping sysclk to ensure we keep the output sck w/in legal bounds
    psram_reinit_timing(F_CPU);
    // Per datasheet, need to do a dummy access and memory barrier before it takes effect
    extern uint8_t __psram_start__;
    volatile uint8_t *x = &__psram_start__;
    *x ^= 0xff;
    *x ^= 0xff;
    asm volatile("" ::: "memory");
#endif

    set_sys_clock_khz(F_CPU / 1000, true);

#if defined(RP2350_PSRAM_CS) && (F_CPU < 150000000)
    psram_reinit_timing();
    // Per datasheet, need to do a dummy access and memory barrier before it takes effect
    extern uint8_t __psram_start__;
    volatile uint8_t *x = &__psram_start__;
    *x ^= 0xff;
    *x ^= 0xff;
    asm volatile("" ::: "memory");
#endif

#endif // over/underclock

    // Let rest of core know if we're using FreeRTOS
    __isFreeRTOS = initFreeRTOS ? true : false;

    // Allocate impure_ptr (newlib temps) if there is a 2nd core running
    if (!__isFreeRTOS && (setup1 || loop1)) {
        _impure_ptr1 = (struct _reent*)calloc(1, sizeof(struct _reent));
        _REENT_INIT_PTR(_impure_ptr1);
    }

    rp2040.begin(0);

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
#if (defined(ENABLE_BLUETOOTH) || defined(ENABLE_BLE)) && defined(DEBUG_RP2040_BLUETOOTH)
        __EnableBluetoothDebug(DEBUG_RP2040_PORT);
#endif
    }
#endif

    if (!__isFreeRTOS) {
        if (setup1 || loop1) {
            rp2040.fifo.begin(2);
        } else {
            rp2040.fifo.begin(1);
        }
        rp2040.fifo.registerCore();
    }
    if (!__isFreeRTOS) {
        if (setup1 || loop1) {
            delay(1); // Needed to make Picoprobe upload start 2nd core
            if (core1_separate_stack) {
                core1_separate_stack_address = (uint32_t*)malloc(0x2000);
                multicore_launch_core1_with_stack(main1, core1_separate_stack_address, 0x2000);
            } else {
                multicore_launch_core1(main1);
            }
        }
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

extern "C" void __register_impure_ptr(struct _reent *p) {
    if (get_core_num() == 0) {
        _impure_ptr = p;
    } else {
        _impure_ptr1 = p;
    }
}

extern "C" struct _reent *__wrap___getreent() __attribute__((weak));
extern "C" struct _reent *__wrap___getreent() {
    if (get_core_num() == 0) {
        return _impure_ptr;
    } else {
        return _impure_ptr1;
    }
}

// ESP8266 internal debug routine
extern void hexdump(const void* mem, uint32_t len, uint8_t cols)  __attribute__((weak));
void hexdump(const void* mem, uint32_t len, uint8_t cols) {
    const char* src = (const char*)mem;
    printf("\n[HEXDUMP] Address: %p len: 0x%lX (%ld)", src, len, len);
    while (len > 0) {
        uint32_t linesize = cols > len ? len : cols;
        printf("\n[%p] 0x%04x: ", src, (int)(src - (const char*)mem));
        for (uint32_t i = 0; i < linesize; i++) {
            printf("%02x ", *(src + i));
        }
        printf("  ");
        for (uint32_t i = linesize; i < cols; i++) {
            printf("   ");
        }
        for (uint32_t i = 0; i < linesize; i++) {
            unsigned char c = *(src + i);
            putc(isprint(c) ? c : '.', stdout);
        }
        src += linesize;
        len -= linesize;
    }
    printf("\n");
}

const String emptyString = "";

extern "C" void __attribute__((__noreturn__)) __wrap___stack_chk_fail() {
    while (true) {
        panic("*** stack smashing detected ***: terminated\n");
    }
}

