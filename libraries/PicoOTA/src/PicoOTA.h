/*
    Pico_OTA.cpp - Programs OTA requests for RP2040 OTA
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

#pragma once

#include <LittleFS.h>
#include <pico_base/pico/ota_command.h>
#include <hardware/resets.h>

extern uint8_t _FS_start;
extern uint8_t _FS_end;

// Simple and low code CRC calculation for the OTA page
class OTACRC32 {
public:
    OTACRC32() {
        crc = 0xffffffff;
    }

    ~OTACRC32() {
    }

    void add(const void *d, uint32_t len) {
        const uint8_t *data = (const uint8_t *)d;
        for (uint32_t i = 0; i < len; i++) {
            crc ^= data[i];
            for (int j = 0; j < 8; j++) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ 0xedb88320;
                } else {
                    crc >>= 1;
                }
            }
        }
    }

    uint32_t get() {
        return ~crc;
    }

private:
    uint32_t crc;
};


// Handles generating valid OTA blocks
class PicoOTA {
public:
    PicoOTA() { }
    ~PicoOTA() { }

    void begin() {
        if (_page) {
            delete _page;
        }
        _page = new OTACmdPage;
        memset(_page, 0, sizeof(*_page));
        memcpy(_page->sign, "Pico OTA", sizeof(_page->sign));
        _page->count = 0;
    }

    bool addFile(const char *filename, uint32_t offset = 0, uint32_t flashaddr = XIP_BASE, uint32_t len = 0) {
        if (!_page  || _page->count == 8) {
            return false;
        }
        File f = LittleFS.open(filename, "r");
        if (!f) {
            return false;
        }
        if (!len) {
            // Check for GZIP header, and if so read real length from file
            uint8_t hdr[4];
            if (2 != f.read(hdr, 2)) {
                // Error, this can't be valid
                f.close();
                return false;
            }
            if ((hdr[0] == 0x1f) && (hdr[1] == 0x8b)) {
                // GZIP signature matched.  Find real size as encoded at the end
                f.seek(f.size() - 4);
                if (4 != f.read(hdr, 4)) {
                    f.close();
                    return false;
                }
                len = hdr[0];
                len += hdr[1] << 8;
                len += hdr[2] << 16;
                len += hdr[3] << 24;
            } else {
                len = f.size();
            }
            len -= offset;
        }
        f.close();
        _page->cmd[_page->count].command = _OTA_WRITE;
        strncpy(_page->cmd[_page->count].write.filename, filename, sizeof(_page->cmd[_page->count].write.filename));
        _page->cmd[_page->count].write.fileOffset = offset;
        _page->cmd[_page->count].write.fileLength = len;
        _page->cmd[_page->count].write.flashAddress = flashaddr;
        _page->count++;
        return true;
    }

    bool commit() {
        if (!_page) {
            return false;
        }

        OTACRC32 crc;
        crc.add(_page, offsetof(OTACmdPage, crc32));
        _page->crc32 = crc.get();

        File f = LittleFS.open(_OTA_COMMAND_FILE, "w");
        if (!f) {
            return false;
        }
        auto len = f.write((uint8_t *)_page, sizeof(*_page));
        f.close();

        return len == sizeof(*_page);
    }

private:
    OTACmdPage *_page = nullptr;
};

extern PicoOTA picoOTA;

