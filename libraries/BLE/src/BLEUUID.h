/*
    BLEUUID - Encapsulate a BT BLE UUID 16/128 bit
    Copyright (c) 2026 Earle F. Philhower, III.  All rights reserved.

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

#include <_needsbt.h>
#include <Arduino.h>

class BLEUUID {
public:
    BLEUUID() {
        is16 = true;
        uuid16 = 0;
    }

    BLEUUID(uint16_t u) {
        is16 = true;
        uuid16 = u;
    }

    BLEUUID(const uint8_t u[16]) {
        is16 = false;
        memcpy(uuid128, u, sizeof(uuid128));
    }

    BLEUUID(String s) {
        is16 = false;
        bzero(uuid128, sizeof(uuid128));
        // 0000FF11-0000-1000-8000-00805F9B34FB
        if (s.length() != 36) {
            return;
        }
        uuid128[15] = readHexByte(s, 34);
        uuid128[14] = readHexByte(s, 32);
        uuid128[13] = readHexByte(s, 30);
        uuid128[12] = readHexByte(s, 28);
        uuid128[11] = readHexByte(s, 26);
        uuid128[10] = readHexByte(s, 24);
        uuid128[9] = readHexByte(s, 21);
        uuid128[8] = readHexByte(s, 19);
        uuid128[7] = readHexByte(s, 16);
        uuid128[6] = readHexByte(s, 14);
        uuid128[5] = readHexByte(s, 11);
        uuid128[4] = readHexByte(s, 9);
        uuid128[3] = readHexByte(s, 6);
        uuid128[2] = readHexByte(s, 4);
        uuid128[1] = readHexByte(s, 2);
        uuid128[0] = readHexByte(s, 0);
    }

    // Does this object hold a "valid" UUID?
    operator bool() {
        if (is16) {
            return uuid16 != 0;
        } else {
            uint8_t x = 0;
            for (size_t i = 0; i < sizeof(uuid128); i++) {
                x |= uuid128[i];
            }
            return x != 0;
        }
    }

    bool operator==(const BLEUUID &r) const {
        if (is16 != r.is16) {
            return false;
        }
        if (is16) {
            return uuid16 == r.uuid16;
        } else {
            return 0 == memcmp(uuid128, r.uuid128, sizeof(uuid128));
        }
    }

    bool operator!=(const BLEUUID &r) const {
        if (is16 != r.is16) {
            return true;
        }
        if (is16) {
            return (uuid16 != r.uuid16);
        } else {
            return !!memcmp(uuid128, r.uuid128, sizeof(uuid128));
        }
    }

    String toString() const {
        char buff[50];
        if (is16) {
            sprintf(buff, "%04x", uuid16);
        } else {
            sprintf(buff, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                    uuid128[0], uuid128[1], uuid128[2], uuid128[3], uuid128[4], uuid128[5], uuid128[6], uuid128[7],
                    uuid128[8], uuid128[9], uuid128[10], uuid128[11], uuid128[12], uuid128[13], uuid128[14], uuid128[15]);
        }
        return String(buff);
    }

    // Write into memory, must ensure destination can handle 16 bytes for uuid128
    size_t write(uint8_t *dst) {
        if (is16) {
            dst[0] = uuid16 & 0xff;
            dst[1] = uuid16 >> 8;
            return 2;
        } else {
            memcpy(dst, uuid128, sizeof(uuid128));
            return sizeof(uuid128);
        }
    }

    bool is16;
    union {
        uint16_t uuid16;
        uint8_t uuid128[16];
    };

private:
    uint8_t readHexByte(String &s, size_t off) {
        if (s.length() < off + 1) {
            return 0; // error, EOL
        }
        // Could parse hex directly, but this is not time critical
        char buff[3];
        buff[0] = s.c_str()[off];
        buff[1] = s.c_str()[off + 1];
        buff[2] = 0;
        uint8_t ret = 0;
        sscanf(buff, "%hhx", &ret);
        return ret;
    }
};
