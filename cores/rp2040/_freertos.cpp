/*
    _freertos.cpp - Internal core definitions for FreeRTOS

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

#include "_freertos.h"
#include <pico/mutex.h>
#include <stdlib.h>
#include "Arduino.h"

typedef struct {
    mutex_t *src;
    SemaphoreHandle_t dst;
} FMMap;

static FMMap *_map = nullptr;
SemaphoreHandle_t __get_freertos_mutex_for_ptr(mutex_t *m, bool recursive) {
    if (!_map) {
        _map = (FMMap *)calloc(16, sizeof(FMMap));
    }
    // Pre-existing map
    for (int i = 0; i < 16; i++) {
        if (m == _map[i].src) {
            return _map[i].dst;
        }
    }

    for (int i = 0; i < 16; i++) {
        if (_map[i].src == nullptr) {
            // Make a new mutex
            SemaphoreHandle_t fm;
            if (recursive) {
                fm = _freertos_recursive_mutex_create();
            } else {
                fm = __freertos_mutex_create();
            }
            if (fm == nullptr) {
                return nullptr;
            }

            _map[i].src = m;
            _map[i].dst = fm;
            return fm;
        }
    }
    return nullptr; // Need to make space for more mutex maps!
}
