/*
    FatFS.h - File system wrapper for FatFS
    Copyright (c) 2024 Earle F. Philhower, III.  All rights reserved.

    Based on spiffs_api.h, which is:
    | Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.

    This code was influenced by NodeMCU and Sming libraries, and first version of
    Arduino wrapper written by Hristo Gochkov.

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

#include <limits>
#include <assert.h>
#include "FS.h"
#include "FSImpl.h"
#include "./ff.h"
#include "./diskio.h"
#include <FS.h>

using namespace fs;

namespace fatfs {

class FatFSFileImpl;
class FatFSDirImpl;
class FatFSConfig : public FSConfig {
public:
    static constexpr uint32_t FSId = 0x46617446;

    FatFSConfig(bool autoFormat = true, bool useFTL = true, int dirEntries = 128, int fatCopies = 1) : FSConfig(FSId, autoFormat), _useFTL(useFTL), _dirEntries(dirEntries), _fatCopies(fatCopies) { }

    FatFSConfig setUseFTL(bool val = true) {
        _useFTL = val;
        return *this;
    }

    FatFSConfig setDirEntries(int entries) {
        _dirEntries = entries;
        return *this;
    }

    FatFSConfig setFATCopies(int copies) {
        _fatCopies = copies;
        return *this;
    }

    bool _useFTL;
    uint16_t _dirEntries;
    uint8_t _fatCopies;
};

class FatFSImpl : public FSImpl {
public:
    FatFSImpl() : _mounted(false) {
    }

    FileImplPtr open(const char* path, OpenMode openMode, AccessMode accessMode) override;

    bool exists(const char* path) override {
        if (_mounted) {
            FILINFO fi;
            if (FR_OK == f_stat(path, &fi)) {
                return true;
            }
        }
        return false;
    }

    DirImplPtr openDir(const char* path) override;

    bool rename(const char* pathFrom, const char* pathTo) override {
        return _mounted ? (FR_OK == f_rename(pathFrom, pathTo)) : false;
    }

    bool info(FSInfo& info) override {
        if (!_mounted) {
            DEBUGV("FatFS::info: FS not mounted\n");
            return false;
        }
        FATFS *fs = nullptr;
        DWORD fre_clust = 0;
        f_getfree("", &fre_clust, &fs);
        info.maxOpenFiles = 999; // TODO - not valid
        info.blockSize = 512 * fs->csize;
        info.pageSize = 0; // TODO ?
        info.maxPathLength = 255; // TODO ?
        info.totalBytes = (uint64_t)(fs->n_fatent - 2) * (uint64_t)fs->csize * 512;
        info.usedBytes = info.totalBytes - (uint64_t)fre_clust * fs->csize * 512;
        return true;
    }

    bool remove(const char* path) override {
        return _mounted ? (FR_OK == f_unlink(path)) : false;
    }

    bool mkdir(const char* path) override {
        return _mounted ? (FR_OK == f_mkdir(path)) : false;
    }

    bool rmdir(const char* path) override {
        return _mounted ? (FR_OK == f_unlink(path)) : false;
    }

    bool stat(const char *path, FSStat *st) override {
        if (!_mounted || !path || !path[0]) {
            return false;
        }
        bzero(st, sizeof(*st));
        FILINFO fno;
        if (FR_OK != f_stat(path, &fno)) {
            return false;
        }
        st->size = fno.fsize;
        st->blocksize = 0;
        st->isDir = (fno.fattrib & AM_DIR) == AM_DIR;
        if (st->isDir) {
            st->size = 0;
        }
        st->ctime = FatToTimeT(fno.fdate, fno.ftime);
        st->atime = FatToTimeT(fno.fdate, fno.ftime);
        return true;
    }

    bool setConfig(const FSConfig &cfg) override {
        if ((cfg._type != FatFSConfig::FSId) || _mounted) {
            DEBUGV("FatFS::setConfig: invalid config or already mounted\n");
            return false;
        }
        _cfg = *static_cast<const FatFSConfig *>(&cfg);
        return true;
    }

    bool begin() override;
    void end() override;
    bool format() override;

    // Helper function, takes FAT and makes standard time_t
    static time_t FatToTimeT(uint16_t d, uint16_t t) {
        struct tm tiempo;
        memset(&tiempo, 0, sizeof(tiempo));
        tiempo.tm_sec  = (((int)t) <<  1) & 0x3e;
        tiempo.tm_min  = (((int)t) >>  5) & 0x3f;
        tiempo.tm_hour = (((int)t) >> 11) & 0x1f;
        tiempo.tm_mday = (int)(d & 0x1f);
        tiempo.tm_mon  = ((int)(d >> 5) & 0x0f) - 1;
        tiempo.tm_year = ((int)(d >> 9) & 0x7f) + 80;
        tiempo.tm_isdst = -1;
        return mktime(&tiempo);
    }

    virtual void setTimeCallback(time_t (*cb)(void)) override {
        extern time_t (*__fatfs_timeCallback)(void);
        __fatfs_timeCallback = cb;
    }

    void sync() {
        disk_ioctl(0, CTRL_SYNC, nullptr);
    }

protected:
    friend class FatFileImpl;
    friend class FatFSDirImpl;

    FATFS* getFs() {
        return &_fatfs;
    }

    static int _getFlags(OpenMode openMode, AccessMode accessMode) {
        int mode = 0;
        if (openMode & OM_CREATE) {
            mode |= FA_CREATE_ALWAYS;
        }
        if (openMode & OM_APPEND) {
            mode |= FA_OPEN_APPEND;
        }
        if (openMode & OM_TRUNCATE) {
            mode |= 0; // TODO - no truncate?
        }
        if ((accessMode & (AM_READ | AM_WRITE)) == (AM_READ | AM_WRITE)) {
            mode |= FA_READ | FA_WRITE;
        } else if (accessMode & AM_READ) {
            mode |= FA_READ;
        } else if (accessMode & AM_WRITE) {
            mode |= FA_WRITE;
        }
        return mode;
    }

    FATFS        _fatfs;
    FatFSConfig  _cfg;
    bool         _mounted;
};


class FatFSFileImpl : public FileImpl {
public:
    FatFSFileImpl(FatFSImpl *fs, std::shared_ptr<FIL> fd, const char *name, bool writable)
        : _fs(fs), _fd(fd), _opened(true), _writable(writable) {
        _name = std::shared_ptr<char>(new char[strlen(name) + 1], std::default_delete<char[]>());
        strcpy(_name.get(), name);
    }

    ~FatFSFileImpl() override {
        flush();
        close();
    }

    int availableForWrite() override {
        return 1; // TODO - not implemented? _opened ? _fd->availableSpaceForWrite() : 0;
    }

    size_t write(const uint8_t *buf, size_t size) override {
        if (_opened) {
            UINT bw;
            if (FR_OK == f_write(_fd.get(), buf, size, &bw)) {
                return bw;
            }
        }
        return -1; // some kind of error
    }

    int read(uint8_t* buf, size_t size) override {
        if (_opened) {
            UINT bw;
            if (FR_OK == f_read(_fd.get(), buf, size, &bw)) {
                return bw;
            }
        }
        return -1;
    }

    void flush() override {
        if (_opened) {
            f_sync(_fd.get());
            if (_writable) {
                _fs->sync();
            }
        }
    }

    bool seek(uint32_t pos, SeekMode mode) override {
        if (!_opened) {
            return false;
        }
        switch (mode) {
        case SeekSet:
            return FR_OK == f_lseek(_fd.get(), pos);
        case SeekEnd:
            return FR_OK == f_lseek(_fd.get(), f_size(_fd.get()) - pos); // TODO again, odd from POSIX
        case SeekCur:
            return FR_OK == f_lseek(_fd.get(), f_tell(_fd.get()) + pos);
        default:
            // Should not be hit, we've got an invalid seek mode
            DEBUGV("FatFSFileImpl::seek: invalid seek mode %d\n", mode);
            assert((mode == SeekSet) || (mode == SeekEnd) || (mode == SeekCur)); // Will fail and give meaningful assert message
            return false;
        }
    }

    size_t position() const override {
        return _opened ? f_tell(_fd.get()) : 0;
    }

    size_t size() const override {
        return _opened ? f_size(_fd.get()) : 0;
    }

    bool truncate(uint32_t size) override {
        if (!_opened) {
            DEBUGV("FatFSFileImpl::truncate: file not opened\n");
            return false;
        }
        if (FR_OK == f_lseek(_fd.get(), size)) {
            return FR_OK == f_truncate(_fd.get());
        }
        return false;
    }

    void close() override {
        if (_opened) {
            f_close(_fd.get());
            // f_close does a disk_sync
            _opened = false;
        }
    }

    const char* name() const override {
        if (!_opened) {
            DEBUGV("FatFSFileImpl::name: file not opened\n");
            return nullptr;
        } else {
            const char *p = _name.get();
            const char *slash = strrchr(p, '/');
            // For names w/o any path elements, return directly
            // If there are slashes, return name after the last slash
            // (note that strrchr will return the address of the slash,
            // so need to increment to ckip it)
            return (slash && slash[1]) ? slash + 1 : p;
        }
    }

    const char* fullName() const override {
        return _opened ? _name.get() : nullptr;
    }

    bool isFile() const override {
        if (_opened) {
            FILINFO fno;
            if (FR_OK == f_stat(_name.get(), &fno)) {
                return (fno.fattrib & AM_DIR) ? false : true;
            }
        }
        return false;
    }

    bool isDirectory() const override {
        if (_opened) {
            FILINFO fno;
            if (FR_OK == f_stat(_name.get(), &fno)) {
                return (fno.fattrib & AM_DIR) ? true : false;
            }
        }
        return false;
    }

    time_t getLastWrite() override {
        return getCreationTime(); // TODO - FatFS doesn't seem to report both filetimes
    }

    time_t getCreationTime() override {
        time_t ftime = 0;
        if (_opened && _fd) {
            FILINFO fno;
            if (FR_OK == f_stat(_name.get(), &fno)) {
                ftime = FatFSImpl::FatToTimeT(fno.fdate, fno.ftime);
            }
        }
        return ftime;
    }

protected:
    FatFSImpl*            _fs;
    std::shared_ptr<FIL>  _fd;
    std::shared_ptr<char> _name;
    bool                  _opened;
    bool                  _writable;
};

class FatFSDirImpl : public DirImpl {
public:
    FatFSDirImpl(const String& pattern, FatFSImpl* fs, std::shared_ptr<DIR> dir, const char *dirPath = nullptr)
        : _pattern(pattern), _fs(fs), _dir(dir), _valid(false), _dirPath(nullptr) {
        if (dirPath) {
            _dirPath = std::shared_ptr<char>(new char[strlen(dirPath) + 1], std::default_delete<char[]>());
            strcpy(_dirPath.get(), dirPath);
        }
        f_opendir(dir.get(), dirPath);
    }

    ~FatFSDirImpl() override {
        if (_dir) {
            f_closedir(_dir.get());
        }
    }

    FileImplPtr openFile(OpenMode openMode, AccessMode accessMode) override {
        if (!_valid) {
            return FileImplPtr();
        }
        // MAX_PATH on FAT32 is potentially 260 bytes per most implementations
        char tmpName[260];
        snprintf(tmpName, sizeof(tmpName), "%s%s%s", _dirPath.get() ? _dirPath.get() : "", _dirPath.get() && _dirPath.get()[0] ? "/" : "", _lfn);
        return _fs->open((const char *)tmpName, openMode, accessMode);
    }

    const char* fileName() override {
        if (!_valid) {
            DEBUGV("FatFSDirImpl::fileName: directory not valid\n");
            return nullptr;
        }
        return (const char*) _lfn; //_dirent.name;
    }

    size_t fileSize() override {
        if (!_valid) {
            return 0;
        }

        return _size;
    }

    time_t fileTime() override {
        if (!_valid) {
            return 0;
        }

        return _time;
    }

    time_t fileCreationTime() override {
        if (!_valid) {
            return 0;
        }

        return _creation;
    }

    bool isFile() const override {
        return _valid ? _isFile : false;
    }

    bool isDirectory() const override {
        return _valid ? _isDirectory : false;
    }

    bool next() override {
        const int n = _pattern.length();
        do {
            FILINFO fno;
            if (FR_OK == f_readdir(_dir.get(), &fno) && fno.fname[0]) {
                _valid = true;
                _size = fno.fsize;
                _isFile = fno.fattrib & AM_DIR ? false : true;
                _isDirectory = !_isFile;
                _time = FatFSImpl::FatToTimeT(fno.fdate, fno.ftime);
                _creation = _time;
                strncpy(_lfn, fno.fname, sizeof(_lfn));
            } else {
                _valid = false;
            }
        } while (_valid && strncmp((const char*) _lfn, _pattern.c_str(), n) != 0);
        return _valid;
    }

    bool rewind() override {
        _valid = false;
        return FR_OK == f_rewinddir(_dir.get());
        return false;
    }

protected:
    String                  _pattern;
    FatFSImpl*              _fs;
    std::shared_ptr<DIR>    _dir;
    bool                    _valid;
    char                    _lfn[64];
    time_t                  _time;
    time_t                  _creation;
    std::shared_ptr<char>   _dirPath;
    uint32_t                _size;
    bool                    _isFile;
    bool                    _isDirectory;
};

}; // namespace sdfs

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_FATFS)
extern FS FatFS;
using fatfs::FatFSConfig;
#endif
