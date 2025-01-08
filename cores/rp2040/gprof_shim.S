#if defined(__riscv)
// Originally from https://github.com/sbzpro/riscv-gprof
# define RSIZE 4

.section .text
.align 2
.globl  _mcount
 _mcount:
  addi sp,sp,-4*RSIZE
  sw ra, 3*RSIZE(sp)
  mv  a1,ra
  call _mcount_internal; //jal  _mcount_internal
  lw ra,  3*RSIZE(sp)
  addi sp,sp,4*RSIZE
  ret
#else
/*
 * profiler.S
 *  Implements the gprof profiler arc counting function.
 *  Created on: 06.08.2015
 *      Author: Erich Styger
 *  Modified for RP2040/RP2350 on Dec 3 2024 by Earle F. Philhower, III.
 */
    .syntax unified
    .arch armv7-m
    .cpu cortex-m0plus

    .text
    .thumb
    .thumb_func
    .align 2

.globl __gnu_mcount_nc
.type __gnu_mcount_nc, %function
.section .time_critical

__gnu_mcount_nc:
// LR = to return to
// SP = to-replace-LR with
  push {r0, r1, r2, r3}
  push {lr}

  // Swap 24/0 
  ldr r0, [sp, #20]
  ldr r1, [sp, #0]
  str r0, [sp, #0]
  str r1, [sp, #20]

  mov r1, lr
  ldr r0, [sp, #0]     /* caller - at the top of the stack */
  bl  _mcount_internal        /* when __gnu_mcount_nc is called */
  pop {r0}
  mov lr, r0
  pop {r0, r1, r2, r3}
  pop {pc}

  .end __gnu_mcount_nc
#endif
