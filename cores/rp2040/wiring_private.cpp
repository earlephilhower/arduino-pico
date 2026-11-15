/*
    wiring_private for the Raspberry Pi Pico RP2040

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
#include <CoreMutex.h>
#include <hardware/gpio.h>
#include <hardware/sync.h>
#include "_freertos.h"


// Support nested IRQ disable/re-enable
#ifndef maxIRQs
#define maxIRQs 15
#endif
static uint32_t _irqStackTop[2] = { 0, 0 };
#ifdef __riscv
static uint32_t _irqStack[2][maxIRQs];
#else
// ARM there's only 1 bit valid, so we'll just shift it in and out...
static uint32_t _irqStack[2];
#endif

#ifdef __FREERTOS
static UBaseType_t __savedIrqs[configNUMBER_OF_CORES];
#endif

extern "C" void interrupts() {
#ifdef __FREERTOS
    if (__freeRTOSinitted) {
        if (portGET_CRITICAL_NESTING_COUNT() == 1U && portCHECK_IF_IN_ISR()) {
            taskEXIT_CRITICAL_FROM_ISR(__savedIrqs[portGET_CORE_ID()]);
        } else {
            taskEXIT_CRITICAL();
        }
        return;
    }
#endif
    auto core = get_core_num();
    if (!_irqStackTop[core]) {
        // ERROR
        return;
    }
#ifdef __riscv
    restore_interrupts(_irqStack[core][--_irqStackTop[core]]);
#else
    uint32_t mask = _irqStack[core] & 1;
    _irqStack[core] >>= 1;
    _irqStackTop[core]--;
    restore_interrupts(mask);
#endif
}

extern "C" void noInterrupts() {
#ifdef __FREERTOS
    if (__freeRTOSinitted) {
        if (portGET_CRITICAL_NESTING_COUNT() == 0U && portCHECK_IF_IN_ISR()) {
            __savedIrqs[portGET_CORE_ID()] = taskENTER_CRITICAL_FROM_ISR();
        } else {
            taskENTER_CRITICAL();
        }
        return;
    }
#endif
    auto core = get_core_num();
    if (_irqStackTop[core] == maxIRQs) {
        // ERROR
        panic("IRQ stack overflow");
    }
#ifdef __riscv
    _irqStack[core][_irqStackTop[core]++] = save_and_disable_interrupts();
#else
    _irqStack[core] <<= 1;
    _irqStack[core] |= save_and_disable_interrupts() ? 1 : 0;
    _irqStackTop[core]++;
#endif
}

auto_init_mutex(_irqMutex);
void *_gpioIrqCB[__GPIOCNT] = {};
void *_gpioIrqCBParam[__GPIOCNT] = {};

// Only 1 GPIO IRQ callback for all pins, so we need to look at the pin it's for and
// dispatch to the real callback manually
static void __not_in_flash_func(_gpioInterruptDispatcher)(uint gpio, uint32_t events) {
    (void) events;
    void *param = _gpioIrqCBParam[gpio];
    if(param) {
        voidFuncPtrParam cb = (voidFuncPtrParam)_gpioIrqCB[gpio];
        if(cb) cb(param);
    }else{
        voidFuncPtr cb = (voidFuncPtr)_gpioIrqCB[gpio];
        if(cb) cb();
    }
}

// To be called when appropriately protected w/IRQ and mutex protects
static void _detachInterruptInternal(pin_size_t pin) {
    if (_gpioIrqCB[pin]) {
        gpio_set_irq_enabled(pin, 0x0f /* all */, false);
        _gpioIrqCB[pin] = 0;
    }
}

extern "C" void attachInterrupt(pin_size_t pin, voidFuncPtr callback, PinStatus mode) {
    attachInterruptParam(pin, (voidFuncPtrParam)callback, mode, 0);
}

void attachInterruptParam(pin_size_t pin, voidFuncPtrParam callback, PinStatus mode, void *param) {
    CoreMutex m(&_irqMutex);
    if (!m) {
        return;
    }
    uint32_t events;
    switch (mode) {
    case LOW:     events = 1; break;
    case HIGH:    events = 2; break;
    case FALLING: events = 4; break;
    case RISING:  events = 8; break;
    case CHANGE:  events = 4 | 8; break;
    default:      return;  // ERROR
    }
    noInterrupts();
    _detachInterruptInternal(pin);
    _gpioIrqCB[pin] = (void *)callback;
    _gpioIrqCBParam[pin] = param;
    gpio_set_irq_enabled_with_callback(pin, events, true, _gpioInterruptDispatcher);
    interrupts();
}

extern "C" void detachInterrupt(pin_size_t pin) {
    CoreMutex m(&_irqMutex);
    if (!m) {
        return;
    }

    noInterrupts();
    _detachInterruptInternal(pin);
    interrupts();
}
