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

#ifdef __cplusplus
}
#endif
