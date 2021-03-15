/*
 * Tone for the Raspberry Pi Pico RP2040
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
#include <pico/time.h>
#include <map>

typedef struct {
    pin_size_t pin;
    bool curState;
    repeating_timer_t timer;
    int timeout;
} Tone;

static std::map<pin_size_t, Tone *> _toneMap;

static bool _toneCB(repeating_timer_t *t) {
    Tone *me = (Tone*) t->user_data;
    if (me->timeout == 1) {
        digitalWrite(me->pin, LOW);
        return false; // done
    }
    me->curState = !me->curState;
    digitalWrite(me->pin, me->curState ? HIGH : LOW);
    if (me->timeout) {
        me->timeout--;
    }
    return true;
}

void tone(uint8_t pin, unsigned int frequency, unsigned long duration) {
    if (!frequency) {
        noTone(pin);
        return;
    }
    int us = 1000000 / frequency / 2;
    if (us < 20) {
        us = 20;
    }
    int flips = duration * 1000 / us;
    auto entry = _toneMap.find(pin);
    if (entry != _toneMap.end()) {
        noTone(pin);
    }
    auto newTone = new Tone();
    newTone->timeout = flips;
    newTone->pin = pin;
    newTone->curState = true;
    _toneMap.insert({pin, newTone});
    digitalWrite(pin, HIGH);
    add_repeating_timer_us(-us, _toneCB, (void*)newTone, &newTone->timer);
}

void noTone(uint8_t pin) {
    auto entry = _toneMap.find(pin);
    if (entry != _toneMap.end()) {
        cancel_repeating_timer(&entry->second->timer);
        _toneMap.erase(entry);
    }
    digitalWrite(pin, LOW);
}
