/*
    Newlib allocator wrapper for multicore fixes

    File a .C and not .CPP because auto_init_recursive_mutex() does not
    compile under C++ for some reason.

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

#include "pico.h"
#include "pico/mutex.h"

auto_init_recursive_mutex(__malloc_reent_mutex);

extern void *__real__malloc_r(void *reent, size_t size);
extern void *__real__calloc_r(void *reent, size_t count, size_t size);
extern void *__real__realloc_r(void *reent, void *mem, size_t size);
extern void __real__free_r(void *reent, void *mem);

void *__wrap__malloc_r(void *reent, size_t size) {
    recursive_mutex_enter_blocking(&__malloc_reent_mutex);
    void *rc = __real__malloc_r(reent, size);
    recursive_mutex_exit(&__malloc_reent_mutex);
    return rc;
}

void *__wrap__calloc_r(void *reent, size_t count, size_t size) {
    recursive_mutex_enter_blocking(&__malloc_reent_mutex);
    void *rc = __real__calloc_r(reent, count, size);
    recursive_mutex_exit(&__malloc_reent_mutex);
    return rc;
}

void *__wrap__realloc_r(void *reent, void *mem, size_t size) {
    recursive_mutex_enter_blocking(&__malloc_reent_mutex);
    void *rc = __real__realloc_r(reent, mem, size);
    recursive_mutex_exit(&__malloc_reent_mutex);
    return rc;
}

void __wrap__free_r(void *reent, void *mem) {
    recursive_mutex_enter_blocking(&__malloc_reent_mutex);
    __real__free_r(reent, mem);
    recursive_mutex_exit(&__malloc_reent_mutex);
}
