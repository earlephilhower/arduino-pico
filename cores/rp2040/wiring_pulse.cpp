/*
    pulseIn for the Raspberry Pi Pico RP2040

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
#include <hardware/timer.h>
#include <hardware/gpio.h>

extern "C" unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
    uint64_t start = time_us_64();
    uint64_t abort = start + timeout;

    if (pin >= __GPIOCNT) {
        DEBUGCORE("ERROR: Illegal pin in pulseIn (%d)\n", pin);
        return 0;
    }

    // Wait for deassert, if needed
    while ((!!gpio_get(pin) != !state) && (time_us_64() < abort));
    if (time_us_64() >= abort) {
        return 0;
    }

    // Wait for assert
    while ((!!gpio_get(pin) != !!state) && (time_us_64() < abort));
    uint64_t begin = time_us_64();
    if (begin >= abort) {
        return 0;
    }

    // Wait for deassert
    while ((!!gpio_get(pin) != !state) && (time_us_64() < abort));
    uint64_t end = time_us_64();
    if (end >= abort) {
        return 0;
    }

    return end - begin;
}

extern "C" unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout) {
    return pulseIn(pin, state, timeout);
}
