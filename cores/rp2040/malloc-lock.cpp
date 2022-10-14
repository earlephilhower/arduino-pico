/*
    Malloc/etc. interrupt locking wrappers

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

#include <Arduino.h>

extern "C" void *__real_malloc(size_t size);
extern "C" void *__real_calloc(size_t count, size_t size);
extern "C" void *__real_realloc(void *mem, size_t size);
extern "C" void __real_free(void *mem);

extern "C" void *__wrap_malloc(size_t size) {
    noInterrupts();
    void *rc = __real_malloc(size);
    interrupts();
    return rc;
}

extern "C" void *__wrap_calloc(size_t count, size_t size) {
    noInterrupts();
    void *rc = __real_calloc(count, size);
    interrupts();
    return rc;
}

extern "C" void *__wrap_realloc(void *mem, size_t size) {
    noInterrupts();
    void *rc = __real_realloc(mem, size);
    interrupts();
    return rc;
}

extern "C" void __wrap_free(void *mem) {
    noInterrupts();
    __real_free(mem);
    interrupts();
}
