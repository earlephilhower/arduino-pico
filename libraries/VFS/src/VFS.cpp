/*
    VFS wrapper to allow POSIX FILE operations

    Copyright (c) 2024 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include <list>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <FSImpl.h>
#include <FS.h>
#include "VFS.h"

// Global static to allow non-class POSIX calls to use this info
typedef struct {
    const char *path;
    FS *fs;
} Entry;
static FS *root = nullptr;
static std::list<Entry> mounts;
static std::map<int, File> files;
static int fd = 3;

VFSClass::VFSClass() {
}


void VFSClass::root(FS &fs) {
    ::root = &fs;
}

void VFSClass::map(const char *path, FS &fs) {
    Entry e = { strdup(path), &fs };
    mounts.push_back(e);
}

static FS *pathToFS(const char **name) {
    const char *nm = *name;
    for (auto a : mounts) {
        if (!strncmp(a.path, nm, strlen(a.path))) {
            *name += strlen(a.path);
            return a.fs;
        }
    }
    return ::root;
}

extern "C" int _open(char *file, int flags, int mode) {
    (void) mode; // No mode RWX here

    const char *nm = file;
    auto fs = pathToFS(&nm);
    if (!fs) {
        return -1;
    }
    const char *md = "r";
    // Taken from table at https://pubs.opengroup.org/onlinepubs/9699919799/functions/fopen.html
    flags &= O_RDONLY | O_WRONLY | O_CREAT | O_TRUNC | O_APPEND | O_RDWR;
    if (flags == O_RDONLY) {
        md = "r";
    } else if (flags == (O_WRONLY | O_CREAT | O_TRUNC)) {
        md = "w";
    } else if (flags == (O_WRONLY | O_CREAT | O_APPEND)) {
        md = "a";
    } else if (flags == O_RDWR) {
        md = "r+";
    } else if (flags == (O_RDWR | O_CREAT | O_TRUNC)) {
        md = "w+";
    } else if (flags == (O_RDWR | O_CREAT | O_APPEND)) {
        md = "a+";
    }
    File f = fs->open(nm, md);
    if (!f) {
        return -1;
    }
    files.insert({fd, f});
    return fd++;
}

extern "C" ssize_t _write(int fd, const void *buf, size_t count) {
#if defined DEBUG_RP2040_PORT
    if (fd < 3) {
        return DEBUG_RP2040_PORT.write((const char *)buf, count);
    }
#endif
    auto f = files.find(fd);
    if (f == files.end()) {
        return 0; // FD not found
    }
    return f->second.write((const char *)buf, count);
}

extern "C" int _close(int fd) {
    auto f = files.find(fd);
    if (f == files.end()) {
        return -1;
    }
    f->second.close();
    files.erase(f);
    return 0;
}

extern "C" int _lseek(int fd, int ptr, int dir) {
    auto f = files.find(fd);
    if (f == files.end()) {
        return -1;
    }
    SeekMode d = SeekSet;
    if (dir == SEEK_CUR) {
        d = SeekCur;
    } else if (dir == SEEK_END) {
        d = SeekEnd;
    }
    return f->second.seek(ptr, d) ? 0 : 1;
}

extern "C" int _read(int fd, char *buf, int size) {
    auto f = files.find(fd);
    if (f == files.end()) {
        return -1; // FD not found
    }
    return f->second.read((uint8_t *)buf, size);
}

extern "C" int _unlink(char *name) {
    auto f = pathToFS((const char **)&name);
    if (f) {
        return f->remove(name) ? 0 : -1;
    }
    return -1;
}

extern "C" int _stat(const char *name, struct stat *st) {
    auto f = pathToFS((const char **)&name);
    if (f) {
        fs::FSStat s;
        if (!f->stat(name, &s)) {
            return -1;
        }
        bzero(st, sizeof(*st));
        st->st_size = s.size;
        st->st_blksize = s.blocksize;
        st->st_ctim.tv_sec = s.ctime;
        st->st_atim.tv_sec = s.atime;
        st->st_mode = s.isDir ? S_IFDIR : S_IFREG;
        return 0;
    }
    return -1;
}

extern "C" int _fstat(int fd, struct stat *st) {
    auto f = files.find(fd);
    if (f == files.end()) {
        return -1; // FD not found
    }
    fs::FSStat s;
    if (!f->second.stat(&s)) {
        return -1;
    }
    bzero(st, sizeof(*st));
    st->st_size = s.size;
    st->st_blksize = s.blocksize;
    st->st_ctim.tv_sec = s.ctime;
    st->st_ctim.tv_sec = s.ctime;
    st->st_mode = s.isDir ? S_IFDIR : S_IFREG;
    return 0;
}

VFSClass VFS;
