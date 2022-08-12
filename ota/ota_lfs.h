/*
    ota_lfs.h - LittleFS+GZIP support for OTA operations
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
#include <stdbool.h>
#include "ota_command.h"

bool lfsMount(uint8_t *start, uint32_t blockSize, uint32_t size);
bool lfsOpen(const char *filename);
bool lfsSeek(uint32_t offset);
uint8_t *lfsRead(uint32_t len);
void lfsClose();

bool lfsReadOTA(OTACmdPage *ota, uint32_t *blockToErase);
void lfsEraseBlock(uint32_t blockToErase);
