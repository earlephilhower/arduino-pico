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
#include "hardware/irq.h"
#include "hardware/structs/scb.h"
#include "hardware/sync.h"
#include "hardware/flash.h"


#include "ota_lfs.h"
#include "ota_command.h"

static OTACmdPage _ota_cmd;

void do_ota() {
    if (memcmp(_ota_command_rom->sign, "Pico OTA", 8)) {
        return; // No signature
    }

    // TODO - check CRC

    if (!_ota_command_rom->count) {
        return;
    }

    // Copy to RAM and erase so we don't program  over and over
    memcpy(&_ota_cmd, _ota_command_rom, sizeof(_ota_cmd));
    int save = save_and_disable_interrupts();
    flash_range_erase((intptr_t)_ota_command_rom - (intptr_t)XIP_BASE, 4096);
    restore_interrupts(save);

    if (!lfsMount(_ota_cmd._start, _ota_cmd._blockSize, _ota_cmd._size)) {
        return;
    }
    for (uint32_t i = 0; i < _ota_cmd.count; i++) {
        switch (_ota_cmd.cmd[i].command) {
            case _OTA_WRITE:
                if (!lfsOpen(_ota_cmd.cmd[i].write.filename)) {
                    return;
                }
                if (!lfsSeek(_ota_cmd.cmd[i].write.fileOffset)) {
                    return;
                }
                uint32_t toRead = _ota_cmd.cmd[i].write.fileLength;
                uint32_t toWrite = _ota_cmd.cmd[i].write.flashAddress;
                while (toRead) {
                    uint32_t len = (toRead < 4096) ? toRead : 4096;
                    uint8_t *p = lfsRead(len);
                    if (!p) {
                        return;
                    }
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
}


#pragma GCC push_options
#pragma GCC optimize("O0")
int main(unsigned char **a, int b) {
    (void) a;
    (void) b;

    do_ota();

    // Reset the interrupt/etc. vectors to the real app.  Will be copied to RAM in app's runtime_init
    scb_hw->vtor = (uint32_t)0x10004000;

    // Jump to it
    register uint32_t* sp asm("sp");
    register uint32_t _sp = *(uint32_t *)0x10004000;
    register void (*fcn)(void) = (void (*)(void)) *(uint32_t *)0x10004004;
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

// Hard assert, avoid puts/etc.
void hard_assertion_failure() {
    while (1) continue;
}
