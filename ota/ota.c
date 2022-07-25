#include <stdlib.h>
#include <stdint.h>
#include "pico/stdlib.h"
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
#include "../libraries/LittleFS/lib/littlefs/lfs.h"
#include "../libraries/LittleFS/lib/littlefs/lfs_util.h"

#include "ota.h"

#pragma GCC push_options
#pragma GCC optimize("O0")


lfs_t       _lfs;
struct lfs_config  _lfs_cfg;

// The actual flash accessing routines
int lfs_flash_read(const struct lfs_config *c, lfs_block_t block,
                   lfs_off_t off, void *buffer, lfs_size_t size);
int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block,
                   lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block);
int lfs_flash_sync(const struct lfs_config *c);

uint8_t *_start;
uint32_t _blockSize;
uint32_t _size;

int lfs_flash_read(const struct lfs_config *c,
                   lfs_block_t block, lfs_off_t off, void *dst, lfs_size_t size) {
    memcpy(dst, _start + (block * _blockSize) + off, size);
    return 0;
}

int lfs_flash_prog(const struct lfs_config *c,
                   lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    uint8_t *addr = _start + (block * _blockSize) + off;
    int save = save_and_disable_interrupts();
    flash_range_program((intptr_t)addr - (intptr_t)XIP_BASE, (const uint8_t *)buffer, size);
    restore_interrupts(save);
    return 0;
}

int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block) {
    uint8_t *addr = _start + (block * _blockSize);
    int save = save_and_disable_interrupts();
    flash_range_erase((intptr_t)addr - (intptr_t)XIP_BASE, _blockSize);
    restore_interrupts(save);
    return 0;
}

int lfs_flash_sync(const struct lfs_config *c) {
    /* NOOP */
    (void) c;
    return 0;
}

void initlfs() {
        memset(&_lfs, 0, sizeof(_lfs));
        memset(&_lfs_cfg, 0, sizeof(_lfs_cfg));
        _lfs_cfg.context = NULL;
        _lfs_cfg.read = lfs_flash_read;
        _lfs_cfg.prog = lfs_flash_prog;
        _lfs_cfg.erase = lfs_flash_erase;
        _lfs_cfg.sync = lfs_flash_sync;
        _lfs_cfg.read_size = 256;
        _lfs_cfg.prog_size = 256;
        _lfs_cfg.block_size =  _blockSize;
        _lfs_cfg.block_count = _blockSize ? _size / _blockSize : 0;
        _lfs_cfg.block_cycles = 16; // TODO - need better explanation
        _lfs_cfg.cache_size = 256;
        _lfs_cfg.lookahead_size = 256;
        _lfs_cfg.read_buffer = NULL;
        _lfs_cfg.prog_buffer = NULL;
        _lfs_cfg.lookahead_buffer = NULL;
        _lfs_cfg.name_max = 0;
        _lfs_cfg.file_max = 0;
        _lfs_cfg.attr_max = 0;
    }

bool mount() {
    initlfs();
    if (!lfs_mount(&_lfs, &_lfs_cfg)) {
        return false;
    }
    lfs_unmount(&_lfs);
}


// OTA flasher
int main(unsigned char **a, int b) {
    (void) a;
    (void) b;
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    for (int i=0; i<10; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }

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

#pragma GCC pop_options
