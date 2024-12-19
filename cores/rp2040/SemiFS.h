/*
    SemiFS.h - File system wrapper for Semihosting ARM
    Copyright (c) 2024 Earle F. Philhower, III.  All rights reserved.

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

#include "Semihosting.h"
#include "FS.h"
#include "FSImpl.h"

using namespace fs;

namespace semifs {

class SemiFSFileImpl;
class SemiFSConfig : public FSConfig {
public:
    static constexpr uint32_t FSId = 0x53454d49;
    SemiFSConfig() : FSConfig(FSId, false) { }
};

class SemiFSFileImpl : public FileImpl {
public:
    SemiFSFileImpl(int fd, const char *name, bool writable)
        : _fd(fd), _opened(true), _writable(writable) {
        _name = std::shared_ptr<char>(new char[strlen(name) + 1], std::default_delete<char[]>());
        strcpy(_name.get(), name);
    }

    ~SemiFSFileImpl() override {
        flush();
        close();
    }

    int availableForWrite() override {
        return 1; // TODO - not implemented? _opened ? _fd->availableSpaceForWrite() : 0;
    }

    size_t write(const uint8_t *buf, size_t size) override {
        if (_opened) {
            uint32_t a[3];
            a[0] = _fd;
            a[1] = (uint32_t)buf;
            a[2] = size;
            return 0 == Semihost(SEMIHOST_SYS_WRITE, a) ? size : -1;
        }
        return -1; // some kind of error
    }

    int read(uint8_t* buf, size_t size) override {
        if (_opened) {
            uint32_t a[3];
            a[0] = _fd;
            a[1] = (uint32_t)buf;
            a[2] = size;
            int ret = Semihost(SEMIHOST_SYS_READ, a);
            if (ret == 0) {
                return size;
            } else if (ret == (int)size) {
                return -1;
            } else {
                return ret;
            }
        }
        return -1;
    }

    void flush() override {
        /* noop */
    }

    bool seek(uint32_t pos, SeekMode mode) override {
        if (!_opened || (mode != SeekSet)) {
            // No seek cur/end in semihost
            return false;
        }
        uint32_t a[2];
        a[0] = _fd;
        a[1] = pos;
        return !Semihost(SEMIHOST_SYS_SEEK, a);
    }

    size_t position() const override {
        return 0; // Not available semihost
    }

    size_t size() const override {
        if (!_opened) {
            return 0;
        }
        uint32_t a;
        a = _fd;
        int ret = Semihost(SEMIHOST_SYS_FLEN, &a);
        if (ret < 0) {
            return 0;
        }
        return ret;
    }

    bool truncate(uint32_t size) override {
        return false; // Not allowed
    }

    void close() override {
        if (_opened) {
            uint32_t a = _fd;
            Semihost(SEMIHOST_SYS_CLOSE, &a);
            _opened = false;
        }
    }

    const char* name() const override {
        if (!_opened) {
            DEBUGV("SemiFSFileImpl::name: file not opened\n");
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
        return _opened; // Could look at ISTTY but that's not the sense here. Just differentiating between dirs and files
    }

    bool isDirectory() const override {
        return false;
    }

    time_t getLastWrite() override {
        return getCreationTime(); // TODO - FatFS doesn't seem to report both filetimes
    }

    time_t getCreationTime() override {
        time_t ftime = 0;
        return ftime;
    }

protected:
    int                   _fd;
    std::shared_ptr<char> _name;
    bool                  _opened;
    bool                  _writable;
};


class SemiFSImpl : public FSImpl {
public:
    SemiFSImpl() {
        /* noop */
    }

    FileImplPtr open(const char* path, OpenMode openMode, AccessMode accessMode) override {
        if (!path || !path[0]) {
            DEBUGV("SemiFSImpl::open() called with invalid filename\n");
            return FileImplPtr();
        }
        // Mode conversion https://developer.arm.com/documentation/dui0471/m/what-is-semihosting-/sys-open--0x01-?lang=en
        int mode = 1; // "rb"
        if (accessMode == AM_READ) {
            mode = 1; // "rb"
        } else if (accessMode == AM_WRITE) {
            if (openMode & OM_APPEND) {
                mode = 9; // "ab";
            } else {
                mode = 5; // "wb";
            }
        } else {
            if (openMode & OM_TRUNCATE) {
                mode = 7; // "w+b";
            } else if (openMode & OM_APPEND) {
                mode = 3; // "r+b"
            } else {
                mode = 11; // "a+b";
            }
        }
        uint32_t a[3];
        a[0] = (uint32_t)path;
        a[1] = mode;
        a[2] = strlen(path);
        int handle = Semihost(SEMIHOST_SYS_OPEN, a);
        if (handle < 0) {
            return FileImplPtr();
        }
        return std::make_shared<SemiFSFileImpl>(handle, path, (accessMode & AM_WRITE) ? true : false);
    }

    bool exists(const char* path) override {
        File f = open(path, OM_DEFAULT, AM_READ);
        return f ? true : false;
    }

    DirImplPtr openDir(const char* path) override {
        // No directories
        return DirImplPtr();
    }

    bool rename(const char* pathFrom, const char* pathTo) override {
        uint32_t a[4];
        a[0] = (uint32_t)pathFrom;
        a[1] = strlen(pathFrom);
        a[2] = (uint32_t)pathTo;
        a[3] = strlen(pathTo);
        return !Semihost(SEMIHOST_SYS_RENAME, a);
    }

    bool info(FSInfo& info) override {
        // Not available
        return false;
    }

    bool remove(const char* path) override {
        uint32_t a[2];
        a[0] = (uint32_t)path;
        a[1] = strlen(path);
        return !Semihost(SEMIHOST_SYS_REMOVE, a);
    }

    bool mkdir(const char* path) override {
        // No mkdir
        return false;
    }

    bool rmdir(const char* path) override {
        // No rmdir
        return false;
    }

    bool stat(const char *path, FSStat *st) override {
        if (!path || !path[0]) {
            return false;
        }
        uint32_t a[3];
        a[0] = (uint32_t)path;
        a[1] = 0; // READ
        a[2] = strlen(path);
        int fn = Semihost(SEMIHOST_SYS_OPEN, a);
        if (fn < 0) {
            return false;
        }
        bzero(st, sizeof(*st));
        a[0] = fn;
        st->size = Semihost(SEMIHOST_SYS_FLEN, a);
        a[0] = fn;
        Semihost(SEMIHOST_SYS_CLOSE, a);
        return true;
    }

    bool setConfig(const FSConfig &cfg) override {
        return true;
    }

    bool begin() override {
        /* noop */
        return true;
    }

    void end() override {
        /* noop */
    }

    bool format() override {
        return false;
    }
};


}; // namespace sdfs

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SEMIFS)
extern FS SemiFS;
using semifs::SemiFSConfig;
#endif
