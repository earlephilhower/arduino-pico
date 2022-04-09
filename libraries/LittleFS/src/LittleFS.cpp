/*
    LittleFS.cpp - Wrapper for LittleFS for RP2040
    Copyright (c) 2021 Earle F. Philhower, III.  All rights reserved.

    Based extensively off of the ESP8266 SPIFFS code, which is
    Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.

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

#include <Arduino.h>
#include <stdlib.h>
#include <algorithm>
#include "LittleFS.h"
#include <hardware/flash.h>
#include <hardware/sync.h>

#ifdef USE_TINYUSB
// For Serial when selecting TinyUSB.  Can't include in the core because Arduino IDE
// will not link in libraries called from the core.  Instead, add the header to all
// the standard libraries in the hope it will still catch some user cases where they
// use these libraries.
// See https://github.com/earlephilhower/arduino-pico/issues/167#issuecomment-848622174
#include <Adafruit_TinyUSB.h>
#endif

extern uint8_t _FS_start;
extern uint8_t _FS_end;

namespace littlefs_impl {

FileImplPtr LittleFSImpl::open(const char* path, OpenMode openMode, AccessMode accessMode) {
    if (!_mounted) {
        DEBUGV("LittleFSImpl::open() called on unmounted FS\n");
        return FileImplPtr();
    }
    if (!path || !path[0]) {
        DEBUGV("LittleFSImpl::open() called with invalid filename\n");
        return FileImplPtr();
    }
    if (!LittleFSImpl::pathValid(path)) {
        DEBUGV("LittleFSImpl::open() called with too long filename\n");
        return FileImplPtr();
    }

    int flags = _getFlags(openMode, accessMode);
    auto fd = std::make_shared<lfs_file_t>();

    if ((openMode & OM_CREATE) && strchr(path, '/')) {
        // For file creation, silently make subdirs as needed.  If any fail,
        // it will be caught by the real file open later on
        char *pathStr = strdup(path);
        if (pathStr) {
            // Make dirs up to the final fnamepart
            char *ptr = strchr(pathStr, '/');
            while (ptr) {
                *ptr = 0;
                lfs_mkdir(&_lfs, pathStr);
                *ptr = '/';
                ptr = strchr(ptr + 1, '/');
            }
        }
        free(pathStr);
    }

    time_t creation = 0;
    if (_timeCallback && (openMode & OM_CREATE)) {
        // O_CREATE means we *may* make the file, but not if it already exists.
        // See if it exists, and only if not update the creation time
        int rc = lfs_file_open(&_lfs, fd.get(), path, LFS_O_RDONLY);
        if (rc == 0) {
            lfs_file_close(&_lfs, fd.get()); // It exists, don't update create time
        } else {
            creation = _timeCallback();  // File didn't exist or otherwise, so we're going to create this time
        }
    }

    int rc = lfs_file_open(&_lfs, fd.get(), path, flags);
    if (rc == LFS_ERR_ISDIR) {
        // To support the SD.openNextFile, a null FD indicates to the LittleFSFile this is just
        // a directory whose name we are carrying around but which cannot be read or written
        return std::make_shared<LittleFSFileImpl>(this, path, nullptr, flags, creation);
    } else if (rc == 0) {
        lfs_file_sync(&_lfs, fd.get());
        return std::make_shared<LittleFSFileImpl>(this, path, fd, flags, creation);
    } else {
        DEBUGV("LittleFSDirImpl::openFile: rc=%d fd=%p path=`%s` openMode=%d accessMode=%d err=%d\n",
               rc, fd.get(), path, openMode, accessMode, rc);
        return FileImplPtr();
    }
}

DirImplPtr LittleFSImpl::openDir(const char *path) {
    if (!_mounted || !path) {
        return DirImplPtr();
    }
    char *pathStr = strdup(path); // Allow edits on our scratch copy
    // Get rid of any trailing slashes
    while (strlen(pathStr) && (pathStr[strlen(pathStr) - 1] == '/')) {
        pathStr[strlen(pathStr) - 1] = 0;
    }
    // At this point we have a name of "blah/blah/blah" or "blah" or ""
    // If that references a directory, just open it and we're done.
    lfs_info info;
    auto dir = std::make_shared<lfs_dir_t>();
    int rc;
    const char *filter = "";
    if (!pathStr[0]) {
        // openDir("") === openDir("/")
        rc = lfs_dir_open(&_lfs, dir.get(), "/");
        filter = "";
    } else if (lfs_stat(&_lfs, pathStr, &info) >= 0) {
        if (info.type == LFS_TYPE_DIR) {
            // Easy peasy, path specifies an existing dir!
            rc = lfs_dir_open(&_lfs, dir.get(), pathStr);
            filter = "";
        } else {
            // This is a file, so open the containing dir
            char *ptr = strrchr(pathStr, '/');
            if (!ptr) {
                // No slashes, open the root dir
                rc = lfs_dir_open(&_lfs, dir.get(), "/");
                filter = pathStr;
            } else {
                // We've got slashes, open the dir one up
                *ptr = 0; // Remove slash, truncate string
                rc = lfs_dir_open(&_lfs, dir.get(), pathStr);
                filter = ptr + 1;
            }
        }
    } else {
        // Name doesn't exist, so use the parent dir of whatever was sent in
        // This is a file, so open the containing dir
        char *ptr = strrchr(pathStr, '/');
        if (!ptr) {
            // No slashes, open the root dir
            rc = lfs_dir_open(&_lfs, dir.get(), "/");
            filter = pathStr;
        } else {
            // We've got slashes, open the dir one up
            *ptr = 0; // Remove slash, truncate string
            rc = lfs_dir_open(&_lfs, dir.get(), pathStr);
            filter = ptr + 1;
        }
    }
    if (rc < 0) {
        DEBUGV("LittleFSImpl::openDir: path=`%s` err=%d\n", path, rc);
        free(pathStr);
        return DirImplPtr();
    }
    // Skip the . and .. entries
    lfs_info dirent;
    lfs_dir_read(&_lfs, dir.get(), &dirent);
    lfs_dir_read(&_lfs, dir.get(), &dirent);

    auto ret = std::make_shared<LittleFSDirImpl>(filter, this, dir, pathStr);
    free(pathStr);
    return ret;
}

int LittleFSImpl::lfs_flash_read(const struct lfs_config *c,
                                 lfs_block_t block, lfs_off_t off, void *dst, lfs_size_t size) {
    LittleFSImpl *me = reinterpret_cast<LittleFSImpl*>(c->context);
    //    Serial.printf(" READ: %p, %d\n", me->_start + (block * me->_blockSize) + off, size);
    memcpy(dst, me->_start + (block * me->_blockSize) + off, size);
    return 0;
}

int LittleFSImpl::lfs_flash_prog(const struct lfs_config *c,
                                 lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    LittleFSImpl *me = reinterpret_cast<LittleFSImpl*>(c->context);
    uint8_t *addr = me->_start + (block * me->_blockSize) + off;
#ifndef USE_FREERTOS
	noInterrupts();
	rp2040.idleOtherCore();
#endif
    //    Serial.printf("WRITE: %p, $d\n", (intptr_t)addr - (intptr_t)XIP_BASE, size);
    flash_range_program((intptr_t)addr - (intptr_t)XIP_BASE, (const uint8_t *)buffer, size);
#ifndef USE_FREERTOS
	rp2040.resumeOtherCore();
	interrupts();
#endif
    return 0;
}

int LittleFSImpl::lfs_flash_erase(const struct lfs_config *c, lfs_block_t block) {
    LittleFSImpl *me = reinterpret_cast<LittleFSImpl*>(c->context);
    uint8_t *addr = me->_start + (block * me->_blockSize);
    //    Serial.printf("ERASE: %p, %d\n", (intptr_t)addr - (intptr_t)XIP_BASE, me->_blockSize);
#ifndef USE_FREERTOS
	noInterrupts();
	rp2040.idleOtherCore();
#endif
    flash_range_erase((intptr_t)addr - (intptr_t)XIP_BASE, me->_blockSize);
#ifndef USE_FREERTOS
	rp2040.resumeOtherCore();
	interrupts();
#endif
    return 0;
}

int LittleFSImpl::lfs_flash_sync(const struct lfs_config *c) {
    /* NOOP */
    (void) c;
    return 0;
}


}; // namespace

FS LittleFS = FS(FSImplPtr(new littlefs_impl::LittleFSImpl(&_FS_start, &_FS_end - &_FS_start, 256, 4096, 16)));

