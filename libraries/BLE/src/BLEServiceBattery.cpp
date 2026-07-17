/*
    BLEServiceBattery - Implements a simple battery service
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


#include <Arduino.h>
#include "BLEServiceBattery.h"
#include <btstack.h>

BLEServiceBattery::BLEServiceBattery() : BLEService(BLEUUID(ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE)) {
    _battLevel = new BLECharacteristic(BLEUUID(ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL), ATT_PROPERTY_READ | ATT_PROPERTY_NOTIFY, "PicoW Battery Level", ATT_SECURITY_NONE, ATT_SECURITY_NONE);
    addCharacteristic(_battLevel);
    _battLevel->setValue((uint16_t)50);
}

BLEServiceBattery::~BLEServiceBattery() {
    delete _battLevel;
}

void BLEServiceBattery::set(int lvl) {
    _battLevel->setValue(lvl);
}
