/*
    wl_types.h - Library for Arduino Wifi shield.
    Copyright (c) 2018 Arduino SA. All rights reserved.
    Copyright (c) 2011-2014 Arduino.  All right reserved.

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
/*
    wl_types.h

    Created on: Jul 30, 2010
        Author: dlafauci
*/

#pragma once

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WL_FAILURE = -1,
    WL_SUCCESS = 1,
} wl_error_code_t;

/* Authentication modes */
enum wl_auth_mode {
    AUTH_MODE_INVALID,
    AUTH_MODE_AUTO,
    AUTH_MODE_OPEN_SYSTEM,
    AUTH_MODE_SHARED_KEY,
    AUTH_MODE_WPA,
    AUTH_MODE_WPA2,
    AUTH_MODE_WPA_PSK,
    AUTH_MODE_WPA2_PSK
};

typedef enum {
    WL_PING_DEST_UNREACHABLE = -1,
    WL_PING_TIMEOUT = -2,
    WL_PING_UNKNOWN_HOST = -3,
    WL_PING_ERROR = -4
} wl_ping_result_t;

#ifdef __cplusplus
}
#endif
