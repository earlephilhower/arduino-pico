/*
    ESP8285/ESP32C3 helper class for the Challenger RP2040 WiFi enabled boards

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

#pragma once

#define DEFAULT_ESP32_BAUDRATE      115200
#define DEFAULT_ESP_BAUDRATE        DEFAULT_ESP32_BAUDRATE

class Challenger2040WiFiClass {
public:
    Challenger2040WiFiClass(HardwareSerial* = &ESP_SERIAL_PORT);
    void doHWReset();
    void runReset();
    void flashReset();
    bool waitForReady();
    bool reset();
    bool isAlive();
    bool changeBaudRate(int);
    void release();
    void setSerial(HardwareSerial*);
    HardwareSerial* getSerial();
private:
    HardwareSerial* _serial;
};

extern Challenger2040WiFiClass Challenger2040WiFi;
