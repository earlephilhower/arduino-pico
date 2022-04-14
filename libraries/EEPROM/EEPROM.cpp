/*
    EEPROM.cpp - RP2040 EEPROM emulation
    Copyright (c) 2021 Earle F. Philhower III. All rights reserved.

    Based on ESP8266 EEPROM library, which is
    Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.

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
#include "EEPROM.h"
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

extern "C" uint8_t _EEPROM_start;

EEPROMClass::EEPROMClass(void)
    : _sector(&_EEPROM_start) {
}

void EEPROMClass::begin(size_t size) {
    if ((size <= 0) || (size > 4096)) {
        size = 4096;
    }

    _size = (size + 255) & (~255);  // Flash writes limited to 256 byte boundaries

    // In case begin() is called a 2nd+ time, don't reallocate if size is the same
    if (_data && size != _size) {
        delete[] _data;
        _data = new uint8_t[size];
    } else if (!_data) {
        _data = new uint8_t[size];
    }

    memcpy(_data, _sector, _size);

    _dirty = false; //make sure dirty is cleared in case begin() is called 2nd+ time
}

bool EEPROMClass::end() {
    bool retval;

    if (!_size) {
        return false;
    }

    retval = commit();
    if (_data) {
        delete[] _data;
    }
    _data = 0;
    _size = 0;
    _dirty = false;

    return retval;
}

uint8_t EEPROMClass::read(int const address) {
    if (address < 0 || (size_t)address >= _size) {
        return 0;
    }
    if (!_data) {
        return 0;
    }

    return _data[address];
}

void EEPROMClass::write(int const address, uint8_t const value) {
    if (address < 0 || (size_t)address >= _size) {
        return;
    }
    if (!_data) {
        return;
    }

    // Optimise _dirty. Only flagged if data written is different.
    uint8_t* pData = &_data[address];
    if (*pData != value) {
        *pData = value;
        _dirty = true;
    }
}

bool EEPROMClass::commit() {
    if (!_size) {
        return false;
    }
    if (!_dirty) {
        return true;
    }
    if (!_data) {
        return false;
    }

    noInterrupts();
    rp2040.idleOtherCore();
    flash_range_erase((intptr_t)_sector - (intptr_t)XIP_BASE, 4096);
    flash_range_program((intptr_t)_sector - (intptr_t)XIP_BASE, _data, _size);
    rp2040.resumeOtherCore();
    interrupts();

    return true;
}

uint8_t * EEPROMClass::getDataPtr() {
    _dirty = true;
    return &_data[0];
}

uint8_t const * EEPROMClass::getConstDataPtr() const {
    return &_data[0];
}

EEPROMClass EEPROM;
