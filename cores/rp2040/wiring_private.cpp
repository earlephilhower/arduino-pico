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
#include <stack>
#include <map>

// Support nested IRQ disable/re-enable
static std::stack<uint32_t> _irqStack[2];

extern "C" void interrupts() {
    if (_irqStack[get_core_num()].empty()) {
        // ERROR
        return;
    }
    auto oldIrqs = _irqStack[get_core_num()].top();
    _irqStack[get_core_num()].pop();
    restore_interrupts(oldIrqs);
}

extern "C" void noInterrupts() {
    _irqStack[get_core_num()].push(save_and_disable_interrupts());
}

// Only 1 GPIO IRQ callback for all pins, so we need to look at the pin it's for and
// dispatch to the real callback manually
auto_init_mutex(_irqMutex);
class CBInfo {
public:
    CBInfo(voidFuncPtr cb) : _cb(cb), _useParam(false), _param(nullptr) { };
    CBInfo(voidFuncPtrParam cbParam, void *param) : _cbParam(cbParam), _useParam(true), _param(param) { };
    void callback() {
        if (_useParam && _cbParam) {
            _cbParam(_param);
        } else if (_cb) {
            _cb();
        }
    }
private:
    union {
        voidFuncPtr _cb;
        voidFuncPtrParam _cbParam;
    };
    bool _useParam;
    void *_param;
};


static std::map<pin_size_t, CBInfo> _map;

void _gpioInterruptDispatcher(uint gpio, uint32_t events) {
    (void) events;
    // Only need to lock around the std::map check, not the whole IRQ callback
    CoreMutex m(&_irqMutex);
    if (m) {
        auto irq = _map.find(gpio);
        if (irq != _map.end()) {
            auto cb = irq->second;
            cb.callback();
        }
    }
}

extern "C" void attachInterrupt(pin_size_t pin, voidFuncPtr callback, PinStatus mode) {
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
    detachInterrupt(pin);
    CBInfo cb(callback);
    _map.insert({pin, cb});
    gpio_set_irq_enabled_with_callback(pin, events, true, _gpioInterruptDispatcher);
    interrupts();
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
    detachInterrupt(pin);
    CBInfo cb(callback, param);
    _map.insert({pin, cb});
    gpio_set_irq_enabled_with_callback(pin, events, true, _gpioInterruptDispatcher);
    interrupts();
}

extern "C" void detachInterrupt(pin_size_t pin) {
    CoreMutex m(&_irqMutex);
    if (!m) {
        return;
    }

    noInterrupts();
    auto irq = _map.find(pin);
    if (irq != _map.end()) {
        gpio_set_irq_enabled(pin, 0x0f /* all */, false);
        _map.erase(pin);
    }
    interrupts();
}
