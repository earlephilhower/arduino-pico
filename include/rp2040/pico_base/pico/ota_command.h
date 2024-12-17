/*
    ota_command.h - OTA stub that copies from LittleFS to flash
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

#include <stdint.h>

#define _OTA_WRITE 1
#define _OTA_VERIFY 1

typedef struct {
    uint32_t command;
    union {
        struct {
            char filename[64];
            uint32_t fileOffset;
            uint32_t fileLength;
            uint32_t flashAddress;   // Normally XIP_BASE
        } write;
    };
} commandEntry;

// Must fit within 4K page
typedef struct {
    uint8_t sign[8]; // "Pico OTA"

    // List of operations
    uint32_t count;
    commandEntry cmd[8];

    uint32_t crc32; // CRC32 over just the contents of this struct, up until just before this value
} OTACmdPage;

#define _OTA_COMMAND_FILE "otacommand.bin"
