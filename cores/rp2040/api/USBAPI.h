/*
  USBAPI.h
  Copyright (c) 2005-2014 Arduino.  All right reserved.

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

#ifndef __USBAPI__
#define __USBAPI__

#include <stdint.h>

namespace arduino {
//================================================================================
//================================================================================
//  Low level API

typedef struct __attribute__((packed))
{
  union {
    uint8_t bmRequestType;
    struct {
      uint8_t direction : 5;
      uint8_t type : 2;
      uint8_t transferDirection : 1;
    };
  };
	uint8_t 	bRequest;
	uint8_t 	wValueL;
	uint8_t 	wValueH;
	uint16_t 	wIndex;
	uint16_t 	wLength;
} USBSetup;

}

//================================================================================
// USB APIs (C scope)
//================================================================================

int USB_SendControl(uint8_t flags, const void* d, int len);
int USB_RecvControl(void* d, int len);
int USB_RecvControlLong(void* d, int len);

uint8_t	USB_Available(uint8_t ep);
uint8_t USB_SendSpace(uint8_t ep);
int USB_Send(uint8_t ep, const void* data, int len);	// blocking
int USB_Recv(uint8_t ep, void* data, int len);		// non-blocking
int USB_Recv(uint8_t ep);							// non-blocking
void USB_Flush(uint8_t ep);

#endif