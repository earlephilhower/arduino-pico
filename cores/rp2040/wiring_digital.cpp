/*
    pinMode and digitalRead/Write for the Raspberry Pi Pico RP2040

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

#include "Arduino.h"
#include <hardware/gpio.h>

static PinMode _pm[30];

extern "C" void pinMode(pin_size_t ulPin, PinMode ulMode) {
    switch (ulMode) {
    case INPUT:
        gpio_init(ulPin);
        gpio_set_dir(ulPin, false);
        gpio_disable_pulls(ulPin);
        break;
    case INPUT_PULLUP:
        gpio_init(ulPin);
        gpio_set_dir(ulPin, false);
        gpio_pull_up(ulPin);
        gpio_put(ulPin, 0);
        break;
    case INPUT_PULLDOWN:
        gpio_init(ulPin);
        gpio_set_dir(ulPin, false);
        gpio_pull_down(ulPin);
        gpio_put(ulPin, 1);
        break;
    case OUTPUT:
    case OUTPUT_4MA:
        gpio_init(ulPin);
        gpio_set_drive_strength(ulPin, GPIO_DRIVE_STRENGTH_4MA);
        gpio_set_dir(ulPin, true);
        break;
    case OUTPUT_2MA:
        gpio_init(ulPin);
        gpio_set_drive_strength(ulPin, GPIO_DRIVE_STRENGTH_2MA);
        gpio_set_dir(ulPin, true);
        break;
    case OUTPUT_8MA:
        gpio_init(ulPin);
        gpio_set_drive_strength(ulPin, GPIO_DRIVE_STRENGTH_8MA);
        gpio_set_dir(ulPin, true);
        break;
    case OUTPUT_12MA:
        gpio_init(ulPin);
        gpio_set_drive_strength(ulPin, GPIO_DRIVE_STRENGTH_12MA);
        gpio_set_dir(ulPin, true);
        break;
    default:
        DEBUGCORE("ERROR: Illegal pinMode mode (%d)\n", ulMode);
        // Error
        return;
    }

    if (ulPin > 29) {
        DEBUGCORE("ERROR: Illegal pin in pinMode (%d)\n", ulPin);
        return;
    }
    _pm[ulPin] = ulMode;
}

extern "C" void digitalWrite(pin_size_t ulPin, PinStatus ulVal) {
    if (ulPin > 29) {
        DEBUGCORE("ERROR: Illegal pin in pinMode (%d)\n", ulPin);
        return;
    }
    gpio_set_function(ulPin, GPIO_FUNC_SIO);
    if (_pm[ulPin] == INPUT_PULLDOWN) {
        if (ulVal == LOW) {
            gpio_set_dir(ulPin, false);
        } else {
            gpio_set_dir(ulPin, true);
        }
    } else if (_pm[ulPin] == INPUT_PULLUP) {
        if (ulVal == HIGH) {
            gpio_set_dir(ulPin, false);
        } else {
            gpio_set_dir(ulPin, true);
        }
    } else {
        gpio_put(ulPin, ulVal == LOW ? 0 : 1);
    }
}

extern "C" PinStatus digitalRead(pin_size_t ulPin) {
    if (ulPin > 29) {
        DEBUGCORE("ERROR: Illegal pin in digitalRead (%d)\n", ulPin);
        return LOW;
    }
    return gpio_get(ulPin) ? HIGH : LOW;
}
