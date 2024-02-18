/*
    FatFS.cpp - file system wrapper for FatFS
    Copyright (c) 2024 Earle F. Philhower, III. All rights reserved.

    Based on spiffs_api.cpp which is:
    | Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.

    This code was influenced by NodeMCU and Sming libraries, and first version of
    Arduino wrapper written by Hristo Gochkov.

    This file is part of the esp8266 core for Arduino environment.

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
#include "FatFS.h"
#include <FS.h>

using namespace fs;


#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_FATFS)
FS FatFS = FS(FSImplPtr(new fatfs::FatFSImpl()));
#endif

namespace fatfs {

// Required to be global because SDFAT doesn't allow a this pointer in it's own time call
time_t (*__fatfs_timeCallback)(void) = nullptr;

FileImplPtr FatFSImpl::open(const char* path, OpenMode openMode, AccessMode accessMode) {
    if (!_mounted) {
        DEBUGV("FatFSImpl::open() called on unmounted FS\n");
        return FileImplPtr();
    }
    if (!path || !path[0]) {
        DEBUGV("FatFSImpl::open() called with invalid filename\n");
        return FileImplPtr();
    }
    BYTE flags = _getFlags(openMode, accessMode);
    if ((openMode && OM_CREATE) && strchr(path, '/')) {
        // For file creation, silently make subdirs as needed.  If any fail,
        // it will be caught by the real file open later on
        char *pathStr = strdup(path);
        if (pathStr) {
            // Make dirs up to the final fnamepart
            char *ptr = strrchr(pathStr, '/');
            if (ptr && ptr != pathStr) { // Don't try to make root dir!
                *ptr = 0;
                f_mkdir(pathStr);
            }
        }
        free(pathStr);
    }
    FIL fd;
    if (FR_OK == f_open(&fd, path, flags)) {
        auto sharedFd = std::make_shared<FIL>(fd);
        return std::make_shared<FatFSFileImpl>(this, sharedFd, path);
    }
    DEBUGV("FatFSImpl::openFile: fd=%p path=`%s` openMode=%d accessMode=%d", &fd, path, openMode, accessMode);
    return FileImplPtr();
}

DirImplPtr FatFSImpl::openDir(const char* path) {
    if (!_mounted) {
        return DirImplPtr();
    }
    char *pathStr = strdup(path); // Allow edits on our scratch copy
    if (!pathStr) {
        // OOM
        return DirImplPtr();
    }
    // Get rid of any trailing slashes
    while (strlen(pathStr) && (pathStr[strlen(pathStr) - 1] == '/')) {
        pathStr[strlen(pathStr) - 1] = 0;
    }
    // At this point we have a name of "/blah/blah/blah" or "blah" or ""
    // If that references a directory, just open it and we're done.
    DIR dirFile;
    FILINFO fno;
    const char *filter = "";
    if (!pathStr[0]) {
        // openDir("") === openDir("/")
        f_opendir(&dirFile, "/");
        filter = "";
    } else if (FR_OK == f_stat(pathStr, &fno)) {
        if (fno.fattrib & AM_DIR) {
            // Easy peasy, path specifies an existing dir!
            f_opendir(&dirFile, pathStr);
            filter = "";
        } else {
            // This is a file, so open the containing dir
            char *ptr = strrchr(pathStr, '/');
            if (!ptr) {
                // No slashes, open the root dir
                f_opendir(&dirFile, "/");
                filter = pathStr;
            } else {
                // We've got slashes, open the dir one up
                *ptr = 0; // Remove slash, truncare string
                f_opendir(&dirFile, pathStr);
                filter = ptr + 1;
            }
        }
    } else {
        // Name doesn't exist, so use the parent dir of whatever was sent in
        // This is a file, so open the containing dir
        char *ptr = strrchr(pathStr, '/');
        if (!ptr) {
            // No slashes, open the root dir
            f_opendir(&dirFile, "/");
            filter = pathStr;
        } else {
            // We've got slashes, open the dir one up
            *ptr = 0; // Remove slash, truncare string
            f_opendir(&dirFile, pathStr);
            filter = ptr + 1;
        }
    }
    // TODO -can this ever happen?
//    if (!dirFile) {
//        DEBUGV("FatFSImpl::openDir: path=`%s`\n", path);
//        return DirImplPtr();
//    }
    auto sharedDir = std::make_shared<DIR>(dirFile);
    auto ret = std::make_shared<FatFSDirImpl>(filter, this, sharedDir, pathStr);
    free(pathStr);
    return ret;
}

bool FatFSImpl::format() {
    if (_mounted) {
        return false;
    }
    BYTE *work = new BYTE[FF_MAX_SS * 8]; /* Work area (larger is better for processing time) */
    MKFS_PARM opt = { FM_FAT, 1, 1, 512, 32 };
    auto ret = f_mkfs("", &opt, work, FF_MAX_SS * 8);
    delete[] work;

    return ret == FR_OK;
}

}; // namespace fatfs
#include "diskio.h"

namespace fatfs {
static uint8_t disk[128 * 1024]; 

DSTATUS disk_status(BYTE p) {
    (void) p;
    return 0;
}

DSTATUS disk_initialize(BYTE p) {
    (void) p;
    return 0;
}

DRESULT disk_read(BYTE p, BYTE *buff, LBA_t sect, UINT count) {
    (void) p;
    memcpy(buff, disk + sect * 512, count * 512);
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    (void) pdrv;
    memcpy(disk + sector * 512, buff, count * 512);
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    (void) pdrv;
    switch(cmd) {
        case CTRL_SYNC:
            // TODO - flush FTL
            return RES_OK;
        case GET_SECTOR_COUNT:
        {
            LBA_t *p = (LBA_t *)buff;
            *p = sizeof(disk) / 512;
            return RES_OK;
        }
        case GET_SECTOR_SIZE:
        {
            WORD *w = (WORD *)buff;
            *w = 512;
            return RES_OK;
        }
        case GET_BLOCK_SIZE:
        {
            DWORD *dw = (DWORD *)buff;
            *dw = 512; // TODO - FTL probably doesn't need to export this, raw flash should
            return RES_OK;
        }
        case CTRL_TRIM:
        {
            LBA_t *lba = (LBA_t *)buff;
            for (unsigned int i = lba[0]; i < lba[1]; i++) {
                bzero(disk + i *512, 512);
            }
            return RES_OK;
        }
        default:
            return RES_PARERR;
    }
}

DWORD get_fattime() {
    time_t now;
    if (fatfs::__fatfs_timeCallback) {
        now = fatfs::__fatfs_timeCallback();
    } else {
        now = time(nullptr);
    }
    struct tm *stm = localtime(&now);
    return (DWORD)(stm->tm_year - 80) << 25 |
       (DWORD)(stm->tm_mon + 1) << 21 |
       (DWORD)stm->tm_mday << 16 |
       (DWORD)stm->tm_hour << 11 |
       (DWORD)stm->tm_min << 5 |
       (DWORD)stm->tm_sec >> 1;
}

}
