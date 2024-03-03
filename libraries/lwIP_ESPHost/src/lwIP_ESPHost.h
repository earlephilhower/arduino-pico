/*
    WiFi <-> LWIP for ESPHost library in RP2040 Core

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
#include "CCtrlWrapper.h"
#include "ESPHost.h"

class ESPHostLwIP : public LwipIntfDev<ESPHost> {
public:

    ESPHostLwIP();

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

private:
    static int initEventCb(CCtrlMsgWrapper *resp);
    static int disconnectEventCb(CCtrlMsgWrapper *resp);

    static bool wifiHwInitialized;
    static ESPHostLwIP* instance;

    bool initHW();
    void onDisconnectEvent();

    uint32_t timeout = 10000;
    uint8_t wifiStatus = WL_NO_MODULE;
    WifiApCfg_t ap;
    bool joined = false;
    SoftApCfg_t softAP;

    std::vector<AccessPoint_t> accessPoints;
}
;
