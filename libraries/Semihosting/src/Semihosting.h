/*
    Semihosting.h - Semihosting for Serial and FS access via GDB
    Copyright (c) 2024 Earle F. Philhower, III.  All rights reserved.

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

// Be sure to only use this library with GDB and to enable the ARM semihosting support
// (gdb) monitor arm semihosting enable

// Input/output will be handled by OpenOCD

// From https://developer.arm.com/documentation/dui0471/g/Semihosting/Semihosting-operations?lang=en
typedef enum {
    SYS_CLOSE = 0x02,
    SYS_CLOCK = 0x10,
    SYS_ELAPSED = 0x30,
    SYS_ERRNO = 0x13,
    SYS_FLEN = 0x0C,
    SYS_GET_CMDLINE = 0x15,
    SYS_HEAPINFO = 0x16,
    SYS_ISERROR = 0x08,
    SYS_ISTTY = 0x09,
    SYS_OPEN = 0x01,
    SYS_READ = 0x06,
    SYS_READC = 0x07,
    SYS_REMOVE = 0x0E,
    SYS_RENAME = 0x0F,
    SYS_SEEK = 0x0A,
    SYS_SYSTEM = 0x12,
    SYS_TICKFREQ = 0x31,
    SYS_TIME = 0x11,
    SYS_TMPNAM = 0x0D,
    SYS_WRITE = 0x05,
    SYS_WRITEC = 0x03,
    SYS_WRITE0 = 0x04
} ARM_SEMIHOST;


// From https://github.com/ErichStyger/mcuoneclipse/blob/master/Examples/MCUXpresso/FRDM-K22F/FRDM-K22F_Semihosting/source/McuSemihost.c
static inline int __attribute__((always_inline)) Semihost(int reason, void *arg) {
    int value;
    __asm volatile(
        "mov r0, %[rsn] \n" /* place semihost operation code into R0 */
        "mov r1, %[arg] \n" /* R1 points to the argument array */
        "bkpt 0xAB      \n" /* call debugger */
        "mov %[val], r0 \n" /* debugger has stored result code in R0 */

        : [val] "=r"(value)  /* outputs */
        : [rsn] "r"(reason), [arg] "r"(arg)   /* inputs */
        : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc" /* clobber */
    );
    return value; /* return result code, stored in R0 */
}

#include "SerialSemi.h"
#include "SemiFS.h"
