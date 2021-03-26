/*
 * wiring_private for the Raspberry Pi Pico RP2040
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
#include <hardware/gpio.h>
#include <hardware/sync.h>
#include <stack>

std::stack<uint32_t> irqStack;

extern "C" void interrupts() {
    if (irqStack.empty()) {
        // ERROR
        return;
    }
    restore_interrupts(irqStack.top());
    irqStack.pop();
}

extern "C" void noInterrupts() {
    irqStack.push(save_and_disable_interrupts());
}

static uint32_t _irqMap = 0;

extern "C" void attachInterrupt(pin_size_t pin, voidFuncPtr callback, PinStatus mode) {
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
    detachInterrupt(pin);
    gpio_set_irq_enabled_with_callback(pin, events, true, (gpio_irq_callback_t)callback);
    _irqMap |= 1<<pin;
    interrupts();
}

extern "C" void detachInterrupt(pin_size_t pin){
    noInterrupts();
    if (_irqMap & (1<<pin)) {
        gpio_set_irq_enabled(pin, 0x0f /* all */, false);
        _irqMap &= ~(1<<pin);
    }
    interrupts();
}
