/*
    BLE - Top level class for managing BLE servers and clients
    Copyright (c) 2026 Earle F. Philhower, III.  All rights reserved.

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

#include <Arduino.h>
#include "BLEAddress.h"
#include "BLEAdvertising.h"
#include "BLEBeacon.h"
#include "BLECharacteristic.h"
#include "BLEServer.h"
#include "BLEService.h"
#include "BLEServiceUART.h"
#include "BLEServiceBattery.h"
#include "BLEUUID.h"
#include <vector>

typedef std::vector<BLEAdvertising> BLEScanReport;

class BLEClass {
public:
    BLEClass();
    ~BLEClass();

    // Start the BLE hardware up
    void begin(String name = String("")) ;

    // Return this BLE device's address
    BLEAddress address();


    // Advertising (31-byte broadcast)
    BLEAdvertising *advertising();
    void startAdvertising();
    void stopAdvertising();


    // All real work happens in a BLEServer or BLEClient

    // Returns the single server instance (multiple services can be supported by single server)
    BLEServer *server();

    // Returns the single client instance
    // BLEClient *client();

    // Scanning is a global property, runs synchronously
    BLEScanReport *scan(int timeoutSec = 5, bool active = true, int intervalms = 100, int windowms = 99);
    void clearScan(); // Free the memory used for scan

    //    bool setAdvertisedDeviceCallbacks(BLEAdvertisedDeviceCallbacks *cb);

private:

    uint16_t readHandler(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
    int writeHandler(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
    void packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size);
    void advertisementHandler(uint8_t *pkt);

    BLEServer *_server = nullptr;
    BLEAddress _addr;
    BLEAdvertising _advertising;
    BLEScanReport _scanResults;

    //    BLEAdvertisedDeviceCallbacks *_scanCB = nullptr;

    /*btstack_packet_callback_registration_t hci_event_callback_registration;*/
    void *hci_event_callback_registration = nullptr;
    /*att_service_handler_t service_handler;*/
    void *service_handler = nullptr;
};

// We have one top-level object
extern BLEClass BLE;
