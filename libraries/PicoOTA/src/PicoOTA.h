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
#include <hardware/flash.h>

extern uint8_t _FS_start;
extern uint8_t _FS_end;

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
        memcpy(_page->sign, "Pico OTA Format\0", sizeof(_page->sign));
        _page->_start = &_FS_start;
        _page->_blockSize = 4096;
        _page->_size = &_FS_end - &_FS_start;
        _page->count = 0;
    }

    bool addFile(const char *filename, uint32_t offset = 0x4000, uint32_t flashaddr = 0x4000,  uint32_t len = 0) {
        if (!_page  || _page->count == 8) {
            return false;
        }
        File f = LittleFS.open(filename, "r");
        if (!f) {
            return false;
        }
        if (!len) {
            len = f.size() - offset;
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

        Serial.printf("\n");
        unsigned char *ptr = (unsigned char *)_page;
        for (size_t i = 0; i < sizeof(*_page); i++) {
            Serial.printf("%02x ", *ptr++);
            if (((1+i)%16) == 0) Serial.printf("\n");
        }
        Serial.printf("\n");

        // Stop all cores and write the command
        noInterrupts();
        rp2040.idleOtherCore();
        flash_range_erase((intptr_t)_ota_command_rom - (intptr_t)XIP_BASE, 4096);
        flash_range_program((intptr_t)_ota_command_rom - (intptr_t)XIP_BASE, (const uint8_t *)_page, 4096);
        rp2040.resumeOtherCore();
        interrupts();
        
        delete _page;
        _page = nullptr;
        return true;
    }

private:
    OTACmdPage *_page = nullptr;
};

extern PicoOTA picoOTA;
