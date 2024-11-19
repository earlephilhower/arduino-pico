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
static uint32_t _irqStack[2][maxIRQs];

extern "C" void interrupts() {
    if (__freeRTOSinitted) {
        __freertos_task_exit_critical();
    } else {
        auto core = get_core_num();
        if (!_irqStackTop[core]) {
            // ERROR
            return;
        }
        restore_interrupts(_irqStack[core][--_irqStackTop[core]]);
    }
}

extern "C" void noInterrupts() {
    if (__freeRTOSinitted) {
        __freertos_task_enter_critical();
    } else {
        auto core = get_core_num();
        if (_irqStackTop[core] == maxIRQs) {
            // ERROR
            panic("IRQ stack overflow");
        }
        _irqStack[core][_irqStackTop[core]++] = save_and_disable_interrupts();
    }
}

auto_init_mutex(_irqMutex);
static uint64_t _gpioIrqEnabled = 0; // Sized to work with RP2350B, 48 GPIOs
static uint64_t _gpioIrqUseParam;
void *_gpioIrqCB[__GPIOCNT];
void *_gpioIrqCBParam[__GPIOCNT];

// Only 1 GPIO IRQ callback for all pins, so we need to look at the pin it's for and
// dispatch to the real callback manually
void _gpioInterruptDispatcher(uint gpio, uint32_t events) {
    (void) events;
    uint64_t mask = 1LL << gpio;
    if (_gpioIrqEnabled & mask) {
        if (_gpioIrqUseParam & mask) {
            voidFuncPtr cb = (voidFuncPtr)_gpioIrqCB[gpio];
            cb();
        } else {
            voidFuncPtrParam cb = (voidFuncPtrParam)_gpioIrqCB[gpio];
            cb(_gpioIrqCBParam[gpio]);
        }
    }
}

// To be called when appropriately protected w/IRQ and mutex protects
static void _detachInterruptInternal(pin_size_t pin) {
    uint64_t mask = 1LL << pin;
    if (_gpioIrqEnabled & mask) {
        gpio_set_irq_enabled(pin, 0x0f /* all */, false);
        _gpioIrqEnabled &= ~mask;
    }
}

extern "C" void attachInterrupt(pin_size_t pin, voidFuncPtr callback, PinStatus mode) {
    CoreMutex m(&_irqMutex);
    if (!m) {
        return;
    }
    uint64_t mask = 1LL << pin;
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
    _gpioIrqEnabled |= mask;
    _gpioIrqUseParam &= ~mask; // No parameter
    _gpioIrqCB[pin] = (void *)callback;
    gpio_set_irq_enabled_with_callback(pin, events, true, _gpioInterruptDispatcher);
    interrupts();
}

void attachInterruptParam(pin_size_t pin, voidFuncPtrParam callback, PinStatus mode, void *param) {
    CoreMutex m(&_irqMutex);
    if (!m) {
        return;
    }
    uint64_t mask = 1LL << pin;
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
    _gpioIrqEnabled |= mask;
    _gpioIrqUseParam &= ~mask; // No parameter
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
