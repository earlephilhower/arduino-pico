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

#pragma once

#include <pico/mutex.h>

#ifdef __cplusplus

class RP2040USB {
public:
    RP2040USB() { }

    // Called by an object at global init time to register a HID device, returns a localID to be mapped using findHIDReportID
    // vidMask is the bits in the VID that should be XOR'd when this device is present.
    // 0 means don't invert anything, OTW select a single bitmask 1<<n.
    uint8_t registerHIDDevice(const uint8_t *descriptor, size_t len, int ordering, uint32_t vidMask);

    // Remove a HID device from the USB descriptor.  Only call after usbDisconnect or results could be unpredictable!
    void unregisterHIDDevice(unsigned int localid);

    // Called by an object at global init time to add a new interface (non-HID, like CDC or Picotool)
    uint8_t registerInterface(int interfaces, const uint8_t *descriptor, size_t len, int ordering, uint32_t vidMask);

    // Remove a USB interface from the USB descriptor.  Only call after usbDisconnect or results could be unpredictable!
    void unregisterInterface(unsigned int localid);

    // Get the USB HID actual report ID from the localid
    uint8_t findHIDReportID(unsigned int localid);

    // Get the USB interface number from the localid
    uint8_t findInterfaceID(unsigned int localid);

    // Register a string for a USB descriptor
    uint8_t registerString(const char *str);

    // Get an unassigned in/cmd or out endpoint number
    uint8_t registerEndpointIn();
    uint8_t registerEndpointOut();
    void unregisterEndpointIn(int ep);
    void unregisterEndpointOut(int ep);

    // Disconnects the USB connection to allow editing the HID/interface list
    void disconnect();

    // Reconnects the USB connection to pick up the new descriptor
    void connect();

    // Override the hardcoded USB VID:PID, product, manufacturer, and serials
    void setVIDPID(uint16_t vid, uint16_t pid);
    void setManufacturer(const char *str);
    void setProduct(const char *str);
    void setSerialNumber(const char *str);

    // Called by main() to init the USB HW/SW.
    void begin();

    // Helper class for HID report sending with wait and timeout
    bool HIDReady();

    // Can't have multiple cores updating the TUSB state in parallel
    mutex_t mutex;

#ifdef __FREERTOS
    volatile bool initted = false;
#endif

private:
};

extern RP2040USB USB;

#endif

