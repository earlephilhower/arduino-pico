/*
    MD5Builder - Simple MD5 hash calculations

    Updated for the Pico by Earle F. Philhower, III

    Modified from the ESP8266 version which is
    Copyright (c) 2015 Hristo Gochkov. All rights reserved.
    This file is part of the esp8266 core for Arduino environment.

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
#include <MD5Builder.h>
#include <memory>

static uint8_t hex_char_to_byte(uint8_t c) {
    return (c >= 'a' && c <= 'f') ? (c - ((uint8_t)'a' - 0xa)) :
           (c >= 'A' && c <= 'F') ? (c - ((uint8_t)'A' - 0xA)) :
           (c >= '0' && c <= '9') ? (c - (uint8_t)'0') : 0;
}

void MD5Builder::begin(void) {
    memset(_buf, 0x00, 16);
    br_md5_init(&_ctx);
}

void MD5Builder::add(const uint8_t * data, const uint16_t len) {
    br_md5_update(&_ctx, data, len);
}

void MD5Builder::addHexString(const char * data) {
    uint16_t i, len = strlen(data);
    auto tmp = std::unique_ptr<uint8_t[]> {new (std::nothrow) uint8_t[len / 2]};

    if (!tmp) {
        return;
    }

    for (i = 0; i < len; i += 2) {
        uint8_t high = hex_char_to_byte(data[i]);
        uint8_t low = hex_char_to_byte(data[i + 1]);
        tmp[i / 2] = (high & 0x0F) << 4 | (low & 0x0F);
    }
    add(tmp.get(), len / 2);
}

bool MD5Builder::addStream(Stream &stream, const size_t maxLen) {
    const int buf_size = 512;
    int maxLengthLeft = maxLen;

    auto buf = std::unique_ptr<uint8_t[]> {new (std::nothrow) uint8_t[buf_size]};

    if (!buf) {
        return false;
    }

    int bytesAvailable = stream.available();
    while ((bytesAvailable > 0) && (maxLengthLeft > 0)) {

        // determine number of bytes to read
        int readBytes = bytesAvailable;
        if (readBytes > maxLengthLeft) {
            readBytes = maxLengthLeft;    // read only until max_len
        }
        if (readBytes > buf_size) {
            readBytes = buf_size;    // not read more the buffer can handle
        }

        // read data and check if we got something
        int numBytesRead = stream.readBytes(buf.get(), readBytes);
        if (numBytesRead < 1) {
            return false;
        }

        // Update MD5 with buffer payload
        br_md5_update(&_ctx, buf.get(), numBytesRead);

        // update available number of bytes
        maxLengthLeft -= numBytesRead;
        bytesAvailable = stream.available();
    }

    return true;
}

void MD5Builder::calculate(void) {
    br_md5_out(&_ctx, _buf);
}

void MD5Builder::getBytes(uint8_t * output) const {
    memcpy(output, _buf, 16);
}

void MD5Builder::getChars(char * output) const {
    for (uint8_t i = 0; i < 16; i++) {
        sprintf(output + (i * 2), "%02x", _buf[i]);
    }
}

String MD5Builder::toString(void) const {
    char out[33];
    getChars(out);
    return String(out);
}
