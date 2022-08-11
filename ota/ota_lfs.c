/*
    ota_lfs.c - LittleFS+GZIP support for OTA operations
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

#include "ota_lfs.h"
#include "ota_command.h"
#include "hardware/sync.h"
#include "hardware/flash.h"

#include "../libraries/LittleFS/lib/littlefs/lfs.h"
#include "../libraries/LittleFS/lib/littlefs/lfs_util.h"
#include "./uzlib/src/uzlib.h"

static lfs_t _lfs;
static struct lfs_config  _lfs_cfg;

static uint8_t *_start;
static uint32_t _blockSize;
static uint32_t _size;

// The actual flash accessing routines
static lfs_block_t _lastBlock;
static int lfs_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *dst, lfs_size_t size) {
    (void) c;
    memcpy(dst, _start + (block * _blockSize) + off, size);
    _lastBlock = block;
    return 0;
}

static int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    (void) c;
    uint8_t *addr = _start + (block * _blockSize) + off;
    int save = save_and_disable_interrupts();
    flash_range_program((intptr_t)addr - (intptr_t)XIP_BASE, (const uint8_t *)buffer, size);
    restore_interrupts(save);
    return 0;
}

static int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block) {
    (void) c;
    uint8_t *addr = _start + (block * _blockSize);
    int save = save_and_disable_interrupts();
    flash_range_erase((intptr_t)addr - (intptr_t)XIP_BASE, _blockSize);
    restore_interrupts(save);
    return 0;
}

void lfsEraseBlock(uint32_t blockToErase) {
    lfs_flash_erase(&_lfs_cfg, blockToErase);
}

static int lfs_flash_sync(const struct lfs_config *c) {
    /* NOOP */
    (void) c;
    return 0;
}

uint8_t _read_buffer[256];
uint8_t _prog_buffer[256];
uint8_t _lookahead_buffer[256];
bool lfsMount(uint8_t *start, uint32_t blockSize, uint32_t size) {
    _start = start;
    _blockSize = blockSize;
    _size = size;

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
    _lfs_cfg.read_buffer = _read_buffer;
    _lfs_cfg.prog_buffer = _prog_buffer;
    _lfs_cfg.lookahead_buffer = _lookahead_buffer;
    _lfs_cfg.name_max = 0;
    _lfs_cfg.file_max = 0;
    _lfs_cfg.attr_max = 0;
    return lfs_mount(&_lfs, &_lfs_cfg) < 0 ? false : true;
}

static bool _gzip = false;
static lfs_file_t _file;

static unsigned char __attribute__((aligned(4))) uzlib_read_buff[4096];
static unsigned char gzip_dict[32768];
static uint8_t _flash_buff[4096]; // no room for this on the stack
static struct uzlib_uncomp m_uncomp;

static uint8_t _ota_buff[256];
static struct lfs_file_config _ota_cfg = { (void *)_ota_buff, NULL, 0 };

static uint8_t _file_buff[256];
static struct lfs_file_config _file_cfg = { (void *)_file_buff, NULL, 0 };

bool lfsReadOTA(OTACmdPage *ota, uint32_t *blockToErase) {
    lfs_file_t f;

    if (lfs_file_opencfg(&_lfs, &f, _OTA_COMMAND_FILE, LFS_O_RDONLY, &_ota_cfg) < 0) {
        return false;
    }
    if (sizeof(*ota) != lfs_file_read(&_lfs, &f, ota, sizeof(*ota))) {
        return false;
    }
    *blockToErase = _lastBlock;
    // No need to close the file, it's open read-only and we're just going to reboot anyway
    //lfs_file_close(&_lfs, &f);
    return true;
}

static int uzlib_read_cb(struct uzlib_uncomp *m) {
    m->source = uzlib_read_buff;
    int len = lfs_file_read(&_lfs, &_file, uzlib_read_buff, sizeof(uzlib_read_buff));
    m->source_limit = uzlib_read_buff + len;
    return *(m->source++);
}

bool lfsOpen(const char *filename) {
    _gzip = false;
    if (lfs_file_opencfg(&_lfs, &_file, filename, LFS_O_RDONLY, &_file_cfg) < 0) {
        return false;
    }
    char b[2];
    if (sizeof(b) != lfs_file_read(&_lfs, &_file, b, sizeof(b))) {
        return false;
    }
    lfs_file_rewind(&_lfs, &_file);
    if ((b[0] == 0x1f) && (b[1] == 0x8b)) {
        uzlib_init();
        m_uncomp.source = NULL;
        m_uncomp.source_limit = NULL;
        m_uncomp.source_read_cb = uzlib_read_cb;
        uzlib_uncompress_init(&m_uncomp, gzip_dict, sizeof(gzip_dict));
        int res = uzlib_gzip_parse_header(&m_uncomp);
        if (res != TINF_OK) {
            lfs_file_rewind(&_lfs, &_file);
            return false; // Error uncompress header read, could have been false alarm
        }
        _gzip = true;

    }
    return true;
}

bool lfsSeek(uint32_t offset) {
    while (offset) {
        uint32_t to_read = (offset > sizeof(_flash_buff)) ? sizeof(_flash_buff) : offset;
        if (!lfsRead(to_read)) {
            return false;
        }
        offset -= to_read;
    }
    return true;
}

uint8_t *lfsRead(uint32_t len) {
    if (!_gzip) {
        int ret = lfs_file_read(&_lfs, &_file, _flash_buff, len);
        return (len == ret) ? _flash_buff : NULL;
    }
    m_uncomp.dest_start = _flash_buff;
    m_uncomp.dest = _flash_buff;
    m_uncomp.dest_limit = _flash_buff + len;
    int res = uzlib_uncompress(&m_uncomp);
    if ((res != TINF_DONE) && (res != TINF_OK)) {
        return NULL;
    }
    return _flash_buff;
}

void lfsClose() {
    // No need to close the file, it's open read-only and we're just going to reboot anyway
    //lfs_file_close(&_lfs, &_file);
}
