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
#include <malloc.h>
#include <reent.h>
#include "psram.h"

extern "C" void *__real_malloc(size_t size);
extern "C" void *__real_calloc(size_t count, size_t size);
extern "C" void *__real_realloc(void *mem, size_t size);
extern "C" void __real_free(void *mem);
extern "C" struct mallinfo __real_mallinfo();

#ifdef RP2350_PSRAM_CS
extern "C" {
    extern uint8_t __psram_start__;
    extern uint8_t __psram_heap_start__;
    void __malloc_lock(struct _reent *ptr);
    void __malloc_unlock(struct _reent *ptr);
    static void *__ram_start = (void *)0x20000000; // TODO - Is there a SDK exposed variable/macro?
}
#endif

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

#ifdef RP2350_PSRAM_CS
// Utilize the existing malloc lock infrastructure and interrupt blocking
// to work with multicore and FreeRTOS
extern "C" void *pmalloc(size_t size) {
    noInterrupts();
    __malloc_lock(__getreent());
    auto rc = __psram_malloc(size);
    __malloc_unlock(__getreent());
    interrupts();
    return rc;
}

extern "C" void *pcalloc(size_t count, size_t size) {
    noInterrupts();
    __malloc_lock(__getreent());
    auto rc = __psram_calloc(count, size);
    __malloc_unlock(__getreent());
    interrupts();
    return rc;
}
#else
// No PSRAM, always fail
extern "C" void *pmalloc(size_t size) {
    (void) size;
    return nullptr;
}

extern "C" void *pcalloc(size_t count, size_t size) {
    (void) count;
    (void) size;
    return nullptr;
}
#endif

extern "C" void *__wrap_realloc(void *mem, size_t size) {
    void *rc;
    noInterrupts();
#ifdef RP2350_PSRAM_CS
    if (mem && (mem < __ram_start)) {
        rc = __psram_realloc(mem, size);
    } else {
        rc = __real_realloc(mem, size);
    }
#else
    rc = __real_realloc(mem, size);
#endif
    interrupts();
    return rc;
}

extern "C" void __wrap_free(void *mem) {
    noInterrupts();
#ifdef RP2350_PSRAM_CS
    if (mem && (mem < __ram_start)) {
        __psram_free(mem);
    } else {
        __real_free(mem);
    }
#else
    __real_free(mem);
#endif
    interrupts();
}

extern "C" struct mallinfo __wrap_mallinfo() {
    noInterrupts();
    __malloc_lock(__getreent());
    auto ret = __real_mallinfo();
    __malloc_unlock(__getreent());
    interrupts();
    return ret;
}
