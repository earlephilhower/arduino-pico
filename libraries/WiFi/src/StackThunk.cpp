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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "StackThunk.h"

extern "C" {

    uint32_t *stack_thunk_ptr = nullptr;
    uint32_t *stack_thunk_top = nullptr;
    uint32_t *stack_thunk_save = nullptr;  /* Saved A1 while in BearSSL */
    uint32_t stack_thunk_refcnt = 0;

    /* Largest stack usage seen in the wild at  6120 */
#define _stackSize (6400/4)
#define _stackPaint 0xdeadbeef

    /* Add a reference, and allocate the stack if necessary */
    void stack_thunk_add_ref() {
        stack_thunk_refcnt++;
        if (stack_thunk_refcnt == 1) {
            // The stack must be in DRAM, or an Soft WDT will follow. Not sure why,
            // maybe too much time is consumed with the non32-bit exception handler.
            // Also, interrupt handling on an IRAM stack would be very slow.
            // Strings on the stack would be very slow to access as well.
            stack_thunk_ptr = (uint32_t *)malloc(_stackSize * sizeof(uint32_t));
            if (!stack_thunk_ptr) {
                // This is a fatal error, stop the sketch
                abort();
            }
            stack_thunk_top = stack_thunk_ptr + _stackSize - 1;
            stack_thunk_save = nullptr;
            stack_thunk_repaint();
        }
    }

    /* Drop a reference, and free stack if no more in use */
    void stack_thunk_del_ref() {
        if (stack_thunk_refcnt == 0) {
            /* Error! */
            return;
        }
        stack_thunk_refcnt--;
        if (!stack_thunk_refcnt) {
            free(stack_thunk_ptr);
            stack_thunk_ptr = nullptr;
            stack_thunk_top = nullptr;
            stack_thunk_save = nullptr;
        }
    }

    void stack_thunk_repaint() {
        for (int i = 0; i < _stackSize; i++) {
            stack_thunk_ptr[i] = _stackPaint;
        }
    }

    /* Simple accessor functions used by postmortem */
    uint32_t stack_thunk_get_refcnt() {
        return stack_thunk_refcnt;
    }

    uint32_t stack_thunk_get_stack_top() {
        return (uint32_t)stack_thunk_top;
    }

    uint32_t stack_thunk_get_stack_bot() {
        return (uint32_t)stack_thunk_ptr;
    }

    uint32_t stack_thunk_get_cont_sp() {
        return (uint32_t)stack_thunk_save;
    }

    /* Return the number of bytes ever used since the stack was created */
    uint32_t stack_thunk_get_max_usage() {
        uint32_t cnt = 0;

        /* No stack == no usage by definition! */
        if (!stack_thunk_ptr) {
            return 0;
        }

        for (cnt = 0; (cnt < _stackSize) && (stack_thunk_ptr[cnt] == _stackPaint); cnt++) {
            /* Noop, all work done in for() */
        }
        return 4 * (_stackSize - cnt);
    }

};
