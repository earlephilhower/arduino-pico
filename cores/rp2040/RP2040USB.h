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

#include <tusb.h>
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

    // TinyUSB callbacks call bare C functions which jump to these
    const uint8_t *tud_descriptor_device_cb();
    const uint8_t *tud_descriptor_configuration_cb(uint8_t index);
    const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid);
    uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance);

#ifdef __FREERTOS
    // Should probably use a semaphore or something, but this works for now
    volatile bool initted = false;
#endif

private:
    // We can't use non-trivial variables to hold the hid, interface, or string lists.  The global
    // initialization where things like the global Keyboard may be called before the non-trivial
    // objects (i.e. no std::vector).

    // Either a USB interface or HID device descriptor, kept in a linked list
    typedef struct Entry {
        const uint8_t *descriptor;
        unsigned int len        : 12;
        unsigned int interfaces : 4;
        unsigned int order      : 18;
        unsigned int localid    : 6;
        uint32_t mask;
        struct Entry *next;
    } Entry;

    // Find-first-set in a 32b quantity.  Not fast, but doesn't need to be
    int ffs(uint32_t v);

    // Add or remove Entry in a linked list, keeping things ordered by ordering
    uint8_t addEntry(Entry **head, int interfaces, const uint8_t *descriptor, size_t len, int ordering, uint32_t vidMask);
    void removeEntry(Entry **head, unsigned int localid);

    // Find the index (HID report ID or USB interface) of a given localid
    unsigned int findID(Entry *head, unsigned int localid);

    // Generate the binary blob for the device descriptor and HID reports
    void setupDescHIDReport();
    void setupUSBDescriptor();

    // Gets a pointer to the HID report structure, optionally returning the size in len
    uint8_t *getDescHIDReport(int *len);

private:
    Entry *_hids = nullptr;
    Entry *_interfaces = nullptr;

    // USB strings kept in a list of pointers.  Can't use std::vector again because of CRT init non-ordering.
    const char **usbd_desc_str = nullptr;
    uint8_t usbd_desc_str_cnt = 0;
    uint8_t usbd_desc_str_alloc = 0;

    // HID report
    unsigned int _hid_interface = (unsigned int) -1;
    uint8_t _hid_endpoint = 0;
    int      _hid_report_len = 0;
    uint8_t *_hid_report     = nullptr;

    // Global USB descriptor
    uint8_t *usbd_desc_cfg = nullptr;
    int usbd_desc_cfg_len = 0;

    // Available bitmask for endpoints, can never be EP 0
    uint32_t _endpointIn = 0xfffffffe;
    uint32_t _endpointOut = 0xfffffffe;

    // Overrides for the USB ID/etc.
    uint16_t _forceVID = 0;
    uint16_t _forcePID = 0;
    uint8_t _forceManuf = 0;
    uint8_t _forceProd = 0;
    uint8_t _forceSerial = 0;

    // USB device descriptor
    tusb_desc_device_t usbd_desc_device;
};

extern RP2040USB USB;

#endif

