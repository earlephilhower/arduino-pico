/*
    ota.c - OTA stub that copies from LittleFS to flash
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
#include <string.h>
#include <hardware/irq.h>
#include <hardware/structs/scb.h>
#include <hardware/sync.h>
#include <hardware/flash.h>
#include <pico/time.h>
#include <hardware/gpio.h>
#include <hardware/uart.h>
#include <hardware/watchdog.h>
#include "ota_lfs.h"
#include "ota_command.h"

//#define DEBUG 1

#ifndef DEBUG
#define uart_putc(a, b)
#define uart_puts(a, b)
#endif

uint8_t **__FS_START__ = (uint8_t **)(XIP_BASE + 0x3000 - 0x10 + 0x0);
uint8_t **__FS_END__   = (uint8_t **)(XIP_BASE + 0x3000 - 0x10 + 0x4);

void dumphex(uint32_t x) {
#ifndef DEBUG
    (void) x;
#else
    uart_puts(uart0, "0x");
    for (int nibble = 7; nibble >= 0; nibble--) {
        uint32_t n = 0x0f & (x >> (nibble * 4));
        char c = n < 10 ? '0' + n : 'A' + n - 10;
        uart_putc(uart0, c);
    }
#endif
}

static OTACmdPage _ota_cmd;

void do_ota() {
    if (*__FS_START__ == *__FS_END__) {
        return;
    }
    if (!lfsMount(*__FS_START__, 4096, *__FS_END__ - *__FS_START__)) {
        uart_puts(uart0, "mount failed\n");
        return;
    }

    // We are very naughty and record the last block read, since it should be the actual data block of the
    // OTA structure.  We'll erase it behind the scenes to avoid bringing in all of LittleFS write infra.
    uint32_t blockToErase;
    if (!lfsReadOTA(&_ota_cmd, &blockToErase)) {
        return;
    }

    if (memcmp(_ota_cmd.sign, "Pico OTA", 8)) {
        return; // No signature
    }

    uint32_t crc = 0xffffffff;
    const uint8_t *data = (const uint8_t *)&_ota_cmd;
    for (uint32_t i = 0; i < offsetof(OTACmdPage, crc32); i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xedb88320;
            } else {
                crc >>= 1;
            }
        }
    }
    crc = ~crc;
    if (crc != _ota_cmd.crc32) {
        uart_puts(uart0, "\ncrc32 mismatch\n");
        return;
    }

    if (!_ota_cmd.count) {
        uart_puts(uart0, "\nno ota count\n");
        return;
    }

    for (uint32_t i = 0; i < _ota_cmd.count; i++) {
        switch (_ota_cmd.cmd[i].command) {
            case _OTA_WRITE:
                uart_puts(uart0, "write: open ");
                uart_puts(uart0, _ota_cmd.cmd[i].write.filename);
                uart_puts(uart0, " = ");
                if (!lfsOpen(_ota_cmd.cmd[i].write.filename)) {
                    uart_puts(uart0, "failed\n");
                    return;
                }
                uart_puts(uart0, "success\n");
                uart_puts(uart0, "seek ");
                dumphex(_ota_cmd.cmd[i].write.fileOffset);
                uart_puts(uart0, " = ");
                if (!lfsSeek(_ota_cmd.cmd[i].write.fileOffset)) {
                    uart_puts(uart0, "failed\n");
                    return;
                }
                uart_puts(uart0, "success\n");
                uint32_t toRead = _ota_cmd.cmd[i].write.fileLength;
                uint32_t toWrite = _ota_cmd.cmd[i].write.flashAddress;

                while (toRead) {
                    uint32_t len = (toRead < 4096) ? toRead : 4096;
                    uint8_t *p = lfsRead(len);
                    if (!p) {
                        uart_puts(uart0, "read failed\n");
                        return;
                    }
                    uart_puts(uart0, "toread = ");
                    dumphex(toRead);
                    uart_puts(uart0, "\ntowrite = ");
                    dumphex(toWrite);
                    uart_puts(uart0, "\n");
                    // Only write pages which differ (i.e. preserve OTA pages unless the OTA shim changes)
                    if (memcmp(p, (void*)toWrite, 4096)) {
                        uart_puts(uart0, "writing\n");
                        int save = save_and_disable_interrupts();
                        flash_range_erase((intptr_t)toWrite - XIP_BASE, 4096);
                        flash_range_program((intptr_t)toWrite - XIP_BASE, (const uint8_t *)p, 4096);
                        restore_interrupts(save);
                    } else {
                        uart_puts(uart0, "identical to flash, skipping\n");
                    }
                    toRead -= len;
                    toWrite += 4096;
                }
                lfsClose();
                break;
            default:
                // TODO - verify
                break;
        }
    }

    uart_puts(uart0, "\nota completed\n");

    // Work completed, erase record.
    lfsEraseBlock(blockToErase);

    // Do a hard reset just in case the start up sequence is not the same
    watchdog_reboot(0, 0, 100);
}


#pragma GCC push_options
#pragma GCC optimize("O0")
int main(int a, unsigned char **b) {
    (void) a;
    (void) b;

#ifdef DEBUG
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
#endif

    do_ota();

    // Reset the interrupt/etc. vectors to the real app.  Will be copied to RAM in app's runtime_init
    scb_hw->vtor = (uint32_t)0x10003000;

    // Jump to it
    register uint32_t* sp asm("sp");
    register uint32_t _sp = *(uint32_t *)0x10003000;
    register void (*fcn)(void) = (void (*)(void)) *(uint32_t *)0x10003004;
    sp = (uint32_t *)_sp;
    fcn();

    // Should never get here!
    return 0;
}
#pragma GCC pop_options

// Clear out some unwanted extra code
int __wrap_atexit(void (*function)(void)) {
    (void) function;
    return 0;
}

// Clear out some unwanted extra code
void __wrap_exit(int status) {
    (void) status;
    while (1) continue;
}

void __wrap_panic(const char *x) {
    (void) x;
    while (1) continue;
}

void __wrap_panic_unsupported() {
    while (1) continue;
}

void __wrap_hard_assertion_failure() {
    while (1) continue;
}
