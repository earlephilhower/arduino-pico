/*
    Core debug macros header for the Raspberry Pi Pico RP2040

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#if !defined(DEBUG_RP2040_PORT)
#define DEBUGV(...) do { } while(0)
#define DEBUGCORE(...) do { } while(0)
#define DEBUGWIRE(...) do { } while(0)
#define DEBUGSPI(...) do { } while(0)
#else
#define DEBUGV(fmt, ...) do { DEBUG_RP2040_PORT.printf(fmt, ## __VA_ARGS__); DEBUG_RP2040_PORT.flush(); } while (0)

#if defined(DEBUG_RP2040_CORE)
#define DEBUGCORE(fmt, ...) do { DEBUG_RP2040_PORT.printf(fmt, ## __VA_ARGS__); DEBUG_RP2040_PORT.flush(); } while (0)
#else
#define DEBUGCORE(...) do { } while(0)
#endif

#if defined(DEBUG_RP2040_WIRE)
#define DEBUGWIRE(fmt, ...) do { DEBUG_RP2040_PORT.printf(fmt, ## __VA_ARGS__); DEBUG_RP2040_PORT.flush(); } while (0)
#else
#define DEBUGWIRE(...) do { } while(0)
#endif

#if defined(DEBUG_RP2040_SPI)
#define DEBUGSPI(fmt, ...) do { DEBUG_RP2040_PORT.printf(fmt, ## __VA_ARGS__); DEBUG_RP2040_PORT.flush(); } while (0)
#else
#define DEBUGSPI(...) do { } while(0)
#endif
#endif

#ifdef __cplusplus
extern void hexdump(const void* mem, uint32_t len, uint8_t cols = 16);
#endif
