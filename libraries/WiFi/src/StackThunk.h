/*
    StackThunk - Implements a simple 2nd stack for BSSL and others
    Copyright (c) 2022 Earle F. Philhower, III.  All rights reserved.

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

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void stack_thunk_add_ref();
extern void stack_thunk_del_ref();
extern void stack_thunk_repaint();

extern uint32_t stack_thunk_get_refcnt();
extern uint32_t stack_thunk_get_stack_top();
extern uint32_t stack_thunk_get_stack_bot();
extern uint32_t stack_thunk_get_cont_sp();
extern uint32_t stack_thunk_get_max_usage();
extern void stack_thunk_dump_stack();
extern void stack_thunk_fatal_overflow();

// Globals required for thunking operation
extern uint32_t *stack_thunk_ptr;
extern uint32_t *stack_thunk_top;
extern uint32_t *stack_thunk_save;
extern uint32_t stack_thunk_refcnt;

#define make_stack_thunk_void(fcnToThunk, proto, params) \
extern "C" void thunk_##fcnToThunk proto { \
    register uint32_t* sp asm("sp"); \
    stack_thunk_save = sp; \
    sp = stack_thunk_top; \
    fcnToThunk params; \
    sp = stack_thunk_save; \
}

#define make_stack_thunk_unsigned_char_ptr(fcnToThunk, proto, params) \
extern "C" unsigned char * thunk_##fcnToThunk proto { \
    register uint32_t* sp asm("sp"); \
    stack_thunk_save = sp; \
    sp = stack_thunk_top; \
    auto x = fcnToThunk params; \
    sp = stack_thunk_save; \
    return x; \
}

#define make_stack_thunk_bool(fcnToThunk, proto, params) \
extern "C" bool thunk_##fcnToThunk proto { \
    register uint32_t* sp asm("sp"); \
    stack_thunk_save = sp; \
    sp = stack_thunk_top; \
    auto x = fcnToThunk params; \
    sp = stack_thunk_save; \
    return x; \
}

#ifdef __cplusplus
}
#endif
