/*
    Shared USB for the Raspberry Pi Pico RP2040
    Allows for multiple endpoints to share the USB controller

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

#include <pico/mutex.h>
#include <limits.h>

// Called by an object at global init time to register a HID device, returns a localID to be mapped using findHIDReportID
// vidMask is the bits in the VID that should be XOR'd when this device is present.
// 0 means don't invert anything, OTW select a single bitmask 1<<n.
uint8_t usbRegisterHIDDevice(const uint8_t *descriptor, size_t len, int ordering, uint32_t vidMask);

// Remove a HID device from the USB descriptor.  Only call after usbDisconnect or results could be unpredictable!
void usbUnregisterHIDDevice(unsigned int localid);

// Called by an object at global init time to add a new interface (non-HID, like CDC or Picotool)
uint8_t usbRegisterInterface(int interfaces, const uint8_t *descriptor, size_t len, int ordering, uint32_t vidMask);

// Remove a USB interface from the USB descriptor.  Only call after usbDisconnect or results could be unpredictable!
void usbUnregisterInterface(unsigned int localid);

// Get the USB HID actual report ID from the localid
uint8_t usbFindHIDReportID(unsigned int localid);

// Get the USB interface number from the localid
uint8_t usbFindInterfaceID(unsigned int localid);

// Register a string for a USB descriptor
uint8_t usbRegisterString(const char *str);

// Get an unassigned in/cmd or out endpoint number
uint8_t usbRegisterEndpointIn();
uint8_t usbRegisterEndpointOut();
void usbUnregisterEndpointIn(int ep);
void usbUnregisterEndpointOut(int ep);

// Disconnects the USB connection to allow editing the HID/interface list
void usbDisconnect();

// Reconnects the USB connection to pick up the new descriptor
void usbConnect();

// Override the hardcoded USB VID:PID, product, manufacturer, and serials
void usbSetVIDPID(uint16_t vid, uint16_t pid);
void usbSetManufacturer(const char *str);
void usbSetProduct(const char *str);
void usbSetSerialNumber(const char *str);

// Big, global USB mutex, shared with all USB devices to make sure we don't
// have multiple cores updating the TUSB state in parallel
extern mutex_t __usb_mutex;

// Called by main() to init the USB HW/SW.
void __USBStart();

// Helper class for HID report sending with wait and timeout
bool __USBHIDReady();
