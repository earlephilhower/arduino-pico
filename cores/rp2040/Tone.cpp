/*
    Tone for the Raspberry Pi Pico RP2040

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
#include "CoreMutex.h"
#include <hardware/gpio.h>
#include <pico/time.h>
#include <map>

typedef struct {
    pin_size_t pin;
    PIO pio;
    int sm;
    alarm_id_t alarm;
} Tone;

// Keep std::map safe for multicore use
auto_init_mutex(_toneMutex);

#include "tone2.pio.h"
static PIOProgram _tone2Pgm(&tone2_program);
static std::map<pin_size_t, Tone *> _toneMap;

int64_t _stopTonePIO(alarm_id_t id, void *user_data) {
    (void) id;
    Tone *tone = (Tone *)user_data;
    tone->alarm = 0;
    pio_sm_set_enabled(tone->pio, tone->sm, false);
    return 0;
}

void tone(uint8_t pin, unsigned int frequency, unsigned long duration) {
    if (pin > 29) {
        DEBUGCORE("ERROR: Illegal pin in tone (%d)\n", pin);
        return;
    }
    if (!frequency) {
        noTone(pin);
        return;
    }

    // Ensure only 1 core can start or stop at a time
    CoreMutex m(&_toneMutex);
    if (!m) {
        return;    // Weird deadlock case
    }

    int us = 1'000'000 / frequency / 2;
    if (us < 5) {
        us = 5;
    }
    auto entry = _toneMap.find(pin);
    Tone *newTone;
    if (entry == _toneMap.end()) {
        newTone = new Tone();
        newTone->pin = pin;
        pinMode(pin, OUTPUT);
        int off;
        if (!_tone2Pgm.prepare(&newTone->pio, &newTone->sm, &off)) {
            DEBUGCORE("ERROR: tone unable to start, out of PIO resources\n");
            // ERROR, no free slots
            delete newTone;
            return;
        }
        tone2_program_init(newTone->pio, newTone->sm, off, pin);
        newTone->alarm = 0;
    } else {
        newTone = entry->second;
        if (newTone->alarm) {
            cancel_alarm(newTone->alarm);
            newTone->alarm = 0;
        }
    }
    pio_sm_clear_fifos(newTone->pio, newTone->sm); // Remove any old updates that haven't yet taken effect
    pio_sm_put_blocking(newTone->pio, newTone->sm, RP2040::usToPIOCycles(us));
    pio_sm_set_enabled(newTone->pio, newTone->sm, true);

    _toneMap.insert({pin, newTone});

    if (duration) {
        auto ret = add_alarm_in_ms(duration, _stopTonePIO, (void *)newTone, true);
        if (ret > 0) {
            newTone->alarm = ret;
        } else {
            DEBUGCORE("ERROR: Unable to allocate timer for tone(%d, %d, %d)\n",
                      pin, frequency, duration);
        }
    }
}

void noTone(uint8_t pin) {
    CoreMutex m(&_toneMutex);

    if ((pin > 29) || !m) {
        DEBUGCORE("ERROR: Illegal pin in tone (%d)\n", pin);
        return;
    }
    auto entry = _toneMap.find(pin);
    if (entry != _toneMap.end()) {
        if (entry->second->alarm) {
            cancel_alarm(entry->second->alarm);
            entry->second->alarm = 0;
        }
        pio_sm_set_enabled(entry->second->pio, entry->second->sm, false);
        pio_sm_unclaim(entry->second->pio, entry->second->sm);
        delete entry->second;
        _toneMap.erase(entry);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
}
