/*
    FatFSUSB - Export onboard FatFS/FTL to host for data movement
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

#ifdef USE_TINYUSB
#error FatFSUSB is not compatible with Adafruit TinyUSB
#endif

#include <Arduino.h>

class FatFSUSBClass {
public:
    FatFSUSBClass();
    ~FatFSUSBClass();

    bool begin();
    void end();

    void driveReady(bool (*cb)(uint32_t), uint32_t cbData = 0);
    void onPlug(void (*cb)(uint32_t), uint32_t cbData = 0);
    void onUnplug(void (*cb)(uint32_t), uint32_t cbData = 0);

    // Only for internal TinyUSB callback use
    bool testUnitReady();
    int32_t read10(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
    int32_t write10(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);
    void plug();;
    void unplug();

private:
    bool _started = false;

    int32_t _sectNum = -1;
    uint8_t *_sectBuff = nullptr;
    uint16_t _sectSize = 0;

    void (*_cbPlug)(uint32_t) = nullptr;
    uint32_t _cbPlugData = 0;

    void (*_cbUnplug)(uint32_t) = nullptr;
    uint32_t _cbUnplugData = 0;

    bool (*_driveReady)(uint32_t) = nullptr;
    uint32_t _driveReadyData = 0;
};

extern FatFSUSBClass FatFSUSB;
