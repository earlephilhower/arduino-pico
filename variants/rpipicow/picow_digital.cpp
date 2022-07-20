/*
    pinMode and digitalRead/Write for the Raspberry Pi Pico W RP2040
    Copyright (c) 2022 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include "Arduino.h"
#include <pico/cyw43_arch.h>

extern "C" void __pinMode(pin_size_t pin, PinMode mode);
extern "C" void __digitalWrite(pin_size_t pin, PinStatus val);
extern "C" PinStatus __digitalRead(pin_size_t pin);

extern "C" void pinMode(pin_size_t pin, PinMode mode) {
    if (pin < 32) {
        __pinMode(pin, mode);
    } else {
        // TBD - There is no GPIO direction control in the driver
    }
}

extern "C" void digitalWrite(pin_size_t pin, PinStatus val) {
    if (pin < 32) {
        __digitalWrite(pin, val);
    } else {
        cyw43_arch_gpio_put(pin - 32, val == HIGH ? 1 : 0);
    }
}

extern "C" PinStatus digitalRead(pin_size_t pin) {
    if (pin < 32) {
        return __digitalRead(pin);
    } else {
        return cyw43_arch_gpio_get(pin - 32) ? HIGH : LOW;
    }
}
