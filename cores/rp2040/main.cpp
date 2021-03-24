/*
 * Main handler for the Raspberry Pi Pico RP2040
 *
 * Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <Arduino.h>

extern void setup();
extern void loop();

extern "C" int main() {
    DEBUG_RP2040_PORT.begin();
    setup();
    while (1) {
        loop();
    }
    return 0;
}


/* NEWLIB syscall overloads, so we can ::print() and ::getc() using our objects */
/* Placed here to ensure this compilation unit will be present when it's time */
/* to link newlib.a. */

#include <stdint.h>
#include <errno.h>
#include <_syslist.h>
#include <sys/times.h>
#include <bits/stdc++.h>
#undef errno


// TODO - check the fd, create a VFS, etc.

extern "C" int errno;

extern "C" ssize_t _write(int fd, const void *buf, size_t count) {
    return DEBUG_RP2040_PORT.write((const char *)buf, count);
}

extern "C" int _chown (const char *path, uid_t owner, gid_t group) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _close (int fildes) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _execve (char  *name, char **argv, char **env) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _fork (void) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _fstat (int          fildes, struct stat *st) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _getpid (void) {
    errno = ENOSYS;
    return -1;
}

struct timeval;

extern "C" int _gettimeofday (struct timeval  *ptimeval, void *ptimezone)
{
    errno = ENOSYS;
    return -1;
}

extern "C" int _isatty (int file) {
    errno = ENOSYS;
    return 0;
}

extern "C" int _kill (int pid, int sig) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _link (char *existing, char *newlink) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _lseek (int   file, int   ptr, int   dir) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _open (char *file, int   flags, int   mode) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _read (int   file, char *ptr, int   len)
{
//    return Serial.read(ptr, len);
return -1;
}

extern "C" int _readlink (const char *path, char *buf, size_t bufsize) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _stat (const char  *file, struct stat *st) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _symlink (const char *path1, const char *path2) {
    errno = ENOSYS;
    return -1;
}

extern "C" clock_t _times (struct tms *buf) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _unlink (char *name) {
    errno = ENOSYS;
    return -1;
}

extern "C" int _wait (int  *status) {
    errno = ENOSYS;
    return -1;
}

extern "C" void __sync_synchronize() { /* noop */ }
