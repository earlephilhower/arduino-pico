/*
    UBlox SARA/ESP helper class for the Connectivity RP2040 LTE/WIFI/BLE board

    Copyright (c) 2024 P. Oldberg <pontus@ilabs.se>

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

#define DEFAULT_SARA_BAUDRATE      115200
#define DEFAULT_ESP_BAUDRATE       115200

class iLabsConnectivityClass {
public:
    iLabsConnectivityClass(HardwareSerial* = &ESP_SERIAL_PORT, HardwareSerial* = &SARA_SERIAL_PORT);

    // Modem stuff
    bool doModemPowerOn();
    bool isModemAlive(uint32_t timeout = 50);
    int getModemMNOProfile();
    bool setModemMNOProfile(int profile);
    bool enableModemPS(bool enable = true);
    String getModemResponse(int timeout = 5);

    // ESP stuff
    void doEspHWReset();
    void runEspReset();
    void flashEspReset();
    bool waitForEspReady();
    bool resetEsp();
    bool isEspAlive();
    bool changeEspBaudRate(int);
    void releaseEsp();
    void setEspSerial(HardwareSerial*);
    HardwareSerial* getEspSerial();


private:
    bool modemSerialPortConfigured;
    bool espSerialPortConfigured;
    HardwareSerial* _espSerial;
    HardwareSerial* _modemSerial;
};

extern iLabsConnectivityClass iLabsConnectivity;
