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



void dumphex(uint32_t x) {
    uart_puts(uart0, "0x");
    for (int nibble = 7; nibble >= 0; nibble--) {
        uint32_t n = 0x0f & (x >> (nibble * 4));
        char c = n < 10 ? '0' + n : 'A' + n - 10;
        uart_putc(uart0, c);
    }
}

static OTACmdPage _ota_cmd;

void do_ota() {
    if (memcmp(_ota_command_rom->sign, "Pico OTA", 8)) {
        uart_puts(uart0, "\nno ota signature\n");
        return; // No signature
    }

    uint32_t crc = 0xffffffff;
    const uint8_t *data = (const uint8_t *)_ota_command_rom;
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
    if (crc != _ota_command_rom->crc32) {
        uart_puts(uart0, "\ncrc32 mismatch\n");
        return;
    }

    if (!_ota_command_rom->count) {
        uart_puts(uart0, "\nno ota count\n");
        return;
    }

    uart_puts(uart0, "\nstarting ota\n");

    // Copy to RAM since theoretically it could be erased by embedded commands
    memcpy(&_ota_cmd, _ota_command_rom, sizeof(_ota_cmd));

    uart_puts(uart0, "\nlfsmount(");
    dumphex((uint32_t)_ota_cmd._start);
    uart_puts(uart0, ",");
    dumphex(_ota_cmd._blockSize);
    uart_puts(uart0, ",");
    dumphex(_ota_cmd._size);
    uart_puts(uart0, ") = ");
    if (!lfsMount(_ota_cmd._start, _ota_cmd._blockSize, _ota_cmd._size)) {
        uart_puts(uart0, "failed\n");
        return;
    }
    uart_puts(uart0, "success\n");
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
                    int save = save_and_disable_interrupts();
                    flash_range_erase((intptr_t)toWrite, 4096);
                    flash_range_program((intptr_t)toWrite, (const uint8_t *)p, 4096);
                    restore_interrupts(save);
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

    // Work completed, erase record.  If we lose power while updating, as long as the bootloader
    // wasn't being updated, then we can retry on the next power up
    int save = save_and_disable_interrupts();
    flash_range_erase((intptr_t)_ota_command_rom - (intptr_t)XIP_BASE, 4096);
    restore_interrupts(save);


    // Do a hard reset just in case the start up sequence is not the same
    watchdog_reboot(0, 0, 100);
}


#pragma GCC push_options
#pragma GCC optimize("O0")
int main(unsigned char **a, int b) {
    (void) a;
    (void) b;

    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);

    do_ota();

    // Reset the interrupt/etc. vectors to the real app.  Will be copied to RAM in app's runtime_init
    scb_hw->vtor = (uint32_t)0x10005000;

    // Jump to it
    register uint32_t* sp asm("sp");
    register uint32_t _sp = *(uint32_t *)0x10005000;
    register void (*fcn)(void) = (void (*)(void)) *(uint32_t *)0x10005004;
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
