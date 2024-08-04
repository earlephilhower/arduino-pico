/*
    SingleFileDrive - Emulates a USB stick for easy data transfer
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

#ifdef USE_TINYUSB
#error SingleFileDrive is not compatible with Adafruit TinyUSB
#endif

#include <Arduino.h>

class SingleFileDrive {
public:
    SingleFileDrive();
    ~SingleFileDrive();

    bool begin(const char *localFile, const char *dosFile);
    void end();

    void onDelete(void (*cb)(uint32_t), uint32_t cbData = 0);
    void onPlug(void (*cb)(uint32_t), uint32_t cbData = 0);
    void onUnplug(void (*cb)(uint32_t), uint32_t cbData = 0);

    // Only for internal TinyUSB callback use
    bool testUnitReady();
    int32_t read10(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
    int32_t write10(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);
    void plug();;
    void unplug();

private:
    void bootSector(char buff[512]);
    void directorySector(char buff[512]);
    void fatSector(char buff[512]);

private:
    bool _started = false;
    char *_localFile = nullptr;
    char *_dosFile = nullptr;

    char _sectBuff[512]; // Read sector region

    void (*_cbDelete)(uint32_t) = nullptr;
    uint32_t _cbDeleteData = 0;

    void (*_cbPlug)(uint32_t) = nullptr;
    uint32_t _cbPlugData = 0;

    void (*_cbUnplug)(uint32_t) = nullptr;
    uint32_t _cbUnplugData = 0;
};

extern SingleFileDrive singleFileDrive;
