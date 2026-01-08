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

    BLEUUID(uint8_t u[16]) {
        is16 = false;
        memcpy(uuid128, u, sizeof(uuid128));
    }

    BLEUUID(const char *str) {
        is16 = false;
        bzero(uuid128, sizeof(uuid128));
        // 0000FF11-0000-1000-8000-00805F9B34FB
        if (strlen(str) != 36) {
            return;
        }
        // Maybe not prettiest or most efficient, but not critical in a constructor
        unsigned int a, b, c, d;
        unsigned long long e;
        if (5 != sscanf(str, "%x-%x-%x-%x-%llx", &a, &b, &c, &d, &e)) {
            return;
        }
        if ((b > 0xffff) || (c > 0xffff) || (d > 0xffff) || (e > 0xffffffffffffll)) {
            return;
        }
        uuid128[15] = e >> 0ll;
        uuid128[14] = e >> 8ll;
        uuid128[13] = e >> 16ll;
        uuid128[12] = e >> 24ll;
        uuid128[11] = e >> 32ll;
        uuid128[10] = e >> 40ll;
        uuid128[9] = d >> 0;
        uuid128[8] = d >> 8;
        uuid128[7] = c >> 0;
        uuid128[6] = c >> 8;
        uuid128[5] = b >> 0;
        uuid128[4] = b >> 8;
        uuid128[3] = a >> 0;
        uuid128[2] = a >> 8;
        uuid128[1] = a >> 16;
        uuid128[0] = a >> 24;
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

    bool operator==(const BLEUUID &r) {
        if (is16 != r.is16) {
            return false;
        }
        if (is16) {
            return uuid16 == r.uuid16;
        } else {
            return 0 == memcmp(uuid128, r.uuid128, sizeof(uuid128));
        }
    }

    String toString() {
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
};
