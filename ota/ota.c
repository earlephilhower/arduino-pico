#include <stdlib.h>
#include <stdint.h>
//#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/irq.h"
#include "hardware/regs/m0plus.h"
#include "hardware/platform_defs.h"
#include "hardware/structs/scb.h"
#include "hardware/claim.h"
#include "hardware/sync.h"
#include "hardware/flash.h"

#include "pico/mutex.h"
#include "pico/assert.h"

#include "ota_lfs.h"

#include "ota_command.h"
#include <string.h>

void do_ota() {
    if (memcmp(_ota_command->sign, "Pico OTA Format\0", 8)) {
        return; // No signature
    }

    // TODO - check CRC

    if (!_ota_command->count) {
        return;
    }

    if (!lfsMount(_ota_command->_start, _ota_command->_blockSize, _ota_command->_size)) {
        return;
    }
    for (uint32_t i = 0; i < _ota_command->count; i++) {
        switch (_ota_command->cmd[i].command) {
            case _OTA_WRITE:
                if (!lfsOpen(_ota_command->cmd[i].write.filename)) {
                    return;
                }
                if (!lfsSeek(_ota_command->cmd[i].write.fileOffset)) {
                    return;
                }
                uint32_t toRead = _ota_command->cmd[i].write.fileLength;
                uint32_t toWrite = _ota_command->cmd[i].write.flashAddress;
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

    // Reset the interrupt/etc. vectors to the real app
    uint32_t *vecsrc = (uint32_t*)0x10004000;
    uint32_t *vecdst = (uint32_t*)0x20000000;
    for (int i=0; i<48; i++) {
        *vecdst++ = *vecsrc++;
    }

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