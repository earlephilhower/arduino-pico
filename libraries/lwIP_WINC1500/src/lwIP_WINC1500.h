/*
    WiFi <-> LWIP for ATWINC!500 in RP2040 Core

    based on Arduino WiFi101 library

    Copyright (c) 2024 Juraj Andrassy

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

#include <LwipIntfDev.h>
#include "winc1500.h"

extern "C" {
#include "driver/include/m2m_wifi.h"
}

class WINC1500LwIP : public LwipIntfDev<WINC1500> {
public:

    WINC1500LwIP();

    void setSTA();
    void setAP();

    void setSSID(const char *p);
    void setBSSID(const uint8_t *bssid);
    void setPassword(const char *p);
    void setChannel(uint8_t channel);

    bool begin();
    void end();
    bool connected();
    uint8_t status();

    uint8_t* macAddress(bool apMode, uint8_t *mac);

    uint8_t* BSSID(uint8_t *bssid);
    int32_t RSSI();
    int channel();
    uint8_t encryptionType();

    int8_t scanNetworks(bool async = false);
    int8_t scanComplete();
    void scanDelete();

    const char* SSID(uint8_t networkItem);
    uint8_t encryptionType(uint8_t networkItem);
    uint8_t* BSSID(uint8_t networkItem, uint8_t *bssid);
    uint8_t channel(uint8_t networkItem);
    int32_t RSSI(uint8_t networkItem);

    void lowPowerMode();
    void noLowPowerMode();

    void setTimeout(int timeout);

    static WINC1500LwIP* instance;
    void handleEvent(uint8_t u8MsgType, void *pvMsg);

private:
    bool initHW();

    bool wifiHwInitialized = false;
    char _version[9];

    bool apMode = false;
    bool joined = false;
    uint32_t _resolve = 0;
    wl_status_t _status = WL_NO_MODULE;
    unsigned long _timeout = 60000;

    tstrM2MConnInfo _connInfo;
    char _pwd[M2M_MAX_PSK_LEN];
    uint8_t _channel = M2M_WIFI_CH_ALL;
    bool requestConnInfo();

    tstrM2mWifiscanResult _scanResult;
    bool requestScanResult(uint8_t networkItem);

    tstrM2MAPConfig _apConfig;
};
