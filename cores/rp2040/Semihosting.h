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

/**
    @brief Semihosting host API opcodes, from https://developer.arm.com/documentation/dui0471/g/Semihosting/Semihosting-operations?lang=en
*/
typedef enum {
    SEMIHOST_SYS_CLOSE = 0x02,
    SEMIHOST_SYS_CLOCK = 0x10,
    SEMIHOST_SYS_ELAPSED = 0x30,
    SEMIHOST_SYS_ERRNO = 0x13,
    SEMIHOST_SYS_FLEN = 0x0C,
    SEMIHOST_SYS_GET_CMDLINE = 0x15,
    SEMIHOST_SYS_HEAPINFO = 0x16,
    SEMIHOST_SYS_ISERROR = 0x08,
    SEMIHOST_SYS_ISTTY = 0x09,
    SEMIHOST_SYS_OPEN = 0x01,
    SEMIHOST_SYS_READ = 0x06,
    SEMIHOST_SYS_READC = 0x07,
    SEMIHOST_SYS_REMOVE = 0x0E,
    SEMIHOST_SYS_RENAME = 0x0F,
    SEMIHOST_SYS_SEEK = 0x0A,
    SEMIHOST_SYS_SYSTEM = 0x12,
    SEMIHOST_SYS_TICKFREQ = 0x31,
    SEMIHOST_SYS_TIME = 0x11,
    SEMIHOST_SYS_TMPNAM = 0x0D,
    SEMIHOST_SYS_WRITE = 0x05,
    SEMIHOST_SYS_WRITEC = 0x03,
    SEMIHOST_SYS_WRITE0 = 0x04
} SEMIHOST_OPCODES;

#ifdef __arm__

/**
    @brief Execute a semihosted request, from https://github.com/ErichStyger/mcuoneclipse/blob/master/Examples/MCUXpresso/FRDM-K22F/FRDM-K22F_Semihosting/source/McuSemihost.c

    @param [in] reason Opcode to execute
    @param [in] arg Any arguments for the opcode
    @returns Result of operation
*/
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
#else

/**
    @brief Execute a semihosted request, from https://groups.google.com/a/groups.riscv.org/g/sw-dev/c/n-5VQ9PHZ4w/m/KbzH5t9MBgAJ

    @param [in] reason Opcode to execute
    @param [in] argPack Any arguments for the opcode
    @returns Result of operation
*/
static inline int __attribute__((always_inline)) Semihost(int reason, void *argPack) {
    register int value asm("a0") = reason;
    register void *ptr asm("a1") = argPack;
    asm volatile(
        // Force 16-byte alignment to make sure that the 3 instructions fall
        // within the same virtual page.
        " .balign 16    \n"
        " .option push \n"
        // Force non-compressed RISC-V instructions
        " .option norvc \n"
        // semihosting e-break sequence
        " slli x0, x0, 0x1f \n" // # Entry NOP
        " ebreak \n"			// # Break to debugger
        " srai x0, x0,  0x7 \n" // # NOP encoding the semihosting call number 7
        " .option pop \n"
        /*mark (value) as an output operand*/
        : "=r"(value) /* Outputs */
        // The semihosting call number is passed in a0, and the argument in a1.
        : "0"(value), "r"(ptr) /* Inputs */
        // The "memory" clobber makes GCC assume that any memory may be arbitrarily read or written by the asm block,
        //  so will prevent the compiler from reordering loads or stores across it, or from caching memory values in registers across it.
        //  The "memory" clobber also prevents the compiler from removing the asm block as dead code.
        : "memory" /* Clobbers */
    );
    return value;
}
#endif
