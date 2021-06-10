/*
  Arduino API main include
  Copyright (c) 2016 Arduino LLC. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef ARDUINO_API_H
#define ARDUINO_API_H

// version 1.3.0
#define ARDUINO_API_VERSION 10300

#include "Binary.h"

#ifdef __cplusplus
#include "Interrupts.h"
#include "IPAddress.h"
#include "Print.h"
#include "Printable.h"
#include "PluggableUSB.h"
#include "Server.h"
#include "String.h"
#include "Stream.h"
#include "Udp.h"
#include "USBAPI.h"
#include "WCharacter.h"
#endif

/* Standard C library includes */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

// Misc Arduino core functions
#include "Common.h"

#ifdef __cplusplus
// Compatibility layer for older code
#include "Compat.h"
#endif

#endif
