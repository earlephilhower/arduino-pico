/*
    ESP8285/ESP32C3 helper class for the iLabs RPICO32 WiFi enabled board

    Copyright (c) 2021,2022 P. Oldberg <pontus@ilabs.se>

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
#include <Ilabs2040WiFiClass.h>

/* Reset the ESP device before the user starts using the device. */
void initVariant() {
    // When the ESP32 co-processor is running ESP-NOW firmware the iLabs_ESP-NOW
    // library owns the reset sequence (see ESP_NOW.setLink()), so skip the AT reset.
#ifndef ILABS_ESPNOW
    Ilabs2040WiFi.reset();
#endif
}
