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

#include "lwIP_ESPHost.h"
#include "CEspControl.h"

#define MAX_SOFTAP_CONNECTION_DEF 5

#if defined(SPIWIFI_ACK) // Arduino Nano RP2040 Connect
#define ESPHOST_DATA_READY SPIWIFI_ACK
#endif

bool ESPHostLwIP::wifiHwInitialized = false;
ESPHostLwIP* ESPHostLwIP::instance = nullptr;

ESPHostLwIP::ESPHostLwIP() :
    LwipIntfDev<ESPHost>(SS, SPI, ESPHOST_DATA_READY) {

}

void ESPHostLwIP::setSTA() {
    apMode = false;
}

void ESPHostLwIP::setAP() {
    apMode = true;
}

void ESPHostLwIP::setSSID(const char *ssid) {
    if (apMode) {
        if (ssid == nullptr) {
            softAP.ssid[0] = 0;
        } else {
            memcpy(softAP.ssid, ssid, sizeof(softAP.ssid));
        }
    } else {
        if (ssid == nullptr) {
            ap.ssid[0] = 0;
        } else {
            memcpy(ap.ssid, ssid, sizeof(ap.ssid));
        }
    }
}

void ESPHostLwIP::setBSSID(const uint8_t *bssid) {
    if (bssid == nullptr || !(bssid[0] | bssid[1] | bssid[2] | bssid[3] | bssid[4] | bssid[5])) {
        ap.bssid[0] = 0;
    } else {
        snprintf((char *)ap.bssid, sizeof(ap.bssid), "%02x:%02x:%02x:%02x:%02x:%02x", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
    }
}

void ESPHostLwIP::setPassword(const char *pass) {
    if (apMode) {
        if (pass == nullptr) {
            softAP.pwd[0] = 0;
        } else {
            memcpy(softAP.pwd, pass, sizeof(softAP.pwd));
        }
    } else {
        if (pass == nullptr) {
            ap.pwd[0] = 0;
        } else {
            memcpy(ap.pwd, pass, sizeof(ap.pwd));
        }
    }
}

void ESPHostLwIP::setChannel(uint8_t channel) {
    softAP.channel = (channel > MAX_CHNL_NO) ? MAX_CHNL_NO : channel;
}

void ESPHostLwIP::setTimeout(int _timeout) {
    timeout = _timeout;
}

int ESPHostLwIP::initEventCb(CCtrlMsgWrapper *resp) {
    (void) resp;
    wifiHwInitialized = true;
    return ESP_CONTROL_OK;
}

int ESPHostLwIP::disconnectEventCb(CCtrlMsgWrapper *resp) {
    (void) resp;
    instance->onDisconnectEvent();
    return ESP_CONTROL_OK;
}

bool ESPHostLwIP::initHW() {
    if (wifiHwInitialized) {
        return true;
    }
    instance = this;

    CEspControl::getInstance().listenForStationDisconnectEvent(disconnectEventCb);
    CEspControl::getInstance().listenForInitEvent(initEventCb);
    if (CEspControl::getInstance().initSpiDriver() != 0) {
        return false;
    }

    uint32_t start = millis();
    while (!wifiHwInitialized && (millis() - start < timeout)) {
        CEspControl::getInstance().communicateWithEsp();
        delay(100);
    }
    if (wifiHwInitialized) {
        wifiStatus = WL_IDLE_STATUS;
    }
    __startEthernetContext();
    return wifiHwInitialized;
}

bool ESPHostLwIP::begin() {
    if (!initHW()) {
        return false;
    }
    ethernet_arch_lwip_gpio_mask();
    if (!apMode) {
        CEspControl::getInstance().setWifiMode(WIFI_MODE_STA);
        if (CEspControl::getInstance().connectAccessPoint(ap) != ESP_CONTROL_OK) {
            wifiStatus = WL_CONNECT_FAILED;
            ethernet_arch_lwip_end();
            return false;
        }
        CEspControl::getInstance().getAccessPointConfig(ap);
    } else {
        CEspControl::getInstance().setWifiMode(WIFI_MODE_AP);
        if (softAP.channel == 0 || softAP.channel > MAX_CHNL_NO) {
            softAP.channel = 1;
        }
        softAP.max_connections = MAX_SOFTAP_CONNECTION_DEF;
        softAP.encryption_mode = softAP.pwd[0] == 0 ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA_WPA2_PSK;
        softAP.bandwidth = WIFI_BW_HT40;
        softAP.ssid_hidden = false;
        int rv = CEspControl::getInstance().startSoftAccessPoint(softAP);
        if (rv != ESP_CONTROL_OK) {
            wifiStatus = WL_AP_FAILED;
            ethernet_arch_lwip_end();
            return false;
        }
        CEspControl::getInstance().getSoftAccessPointConfig(softAP);
    }
    ethernet_arch_lwip_gpio_unmask();
    uint8_t mac[6];
    if (!LwipIntfDev<ESPHost>::begin(macAddress(apMode, mac))) {
        ethernet_arch_lwip_gpio_mask();
        if (apMode) {
            CEspControl::getInstance().stopSoftAccessPoint();
            wifiStatus = WL_AP_FAILED;
        } else {
            CEspControl::getInstance().disconnectAccessPoint();
            wifiStatus = WL_CONNECT_FAILED;
        }
        ethernet_arch_lwip_gpio_unmask();
        return false;
    }
    if (apMode) {
        wifiStatus = WL_AP_LISTENING;
    } else {
        joined = true;
        wifiStatus = LwipIntfDev<ESPHost>::connected() ? WL_CONNECTED : WL_DISCONNECTED;
    }
    return true;
}

void ESPHostLwIP::end() {
    LwipIntfDev<ESPHost>::end();
    if (apMode) {
        CEspControl::getInstance().stopSoftAccessPoint();
    } else {
        CEspControl::getInstance().disconnectAccessPoint();
        joined = false;
    }
    wifiStatus = WL_IDLE_STATUS;
}

void ESPHostLwIP::onDisconnectEvent() {
    LwipIntfDev<ESPHost>::end();
    joined = false;
    wifiStatus = WL_CONNECTION_LOST;
}

bool ESPHostLwIP::connected() {
    return (status() == WL_CONNECTED);
}

uint8_t ESPHostLwIP::status() {
    initHW();
    if (joined && wifiStatus == WL_DISCONNECTED && LwipIntfDev<ESPHost>::connected()) {
        wifiStatus = WL_CONNECTED; // DHCP finished
    }
    return wifiStatus;
}

uint8_t* ESPHostLwIP::macAddress(bool apMode, uint8_t *mac) {
    if (!initHW()) {
        return mac;
    }
    WifiMac_t MAC;
    MAC.mode = apMode ? WIFI_MODE_AP : WIFI_MODE_STA;
    ethernet_arch_lwip_gpio_mask();
    if (CEspControl::getInstance().getWifiMacAddress(MAC) == ESP_CONTROL_OK) {
        CNetUtilities::macStr2macArray(mac, MAC.mac);
    }
    ethernet_arch_lwip_gpio_unmask();
    return mac;
}

uint8_t* ESPHostLwIP::BSSID(uint8_t *bssid) {
    CNetUtilities::macStr2macArray(bssid, (char*) ap.bssid);
    return bssid;
}

int ESPHostLwIP::channel() {
    return ap.channel;
}

int32_t ESPHostLwIP::RSSI() {
    if (!joined) {
        return 0;
    }
    ethernet_arch_lwip_gpio_mask();
    CEspControl::getInstance().getAccessPointConfig(ap);
    ethernet_arch_lwip_gpio_unmask();
    return ap.rssi;
}

static uint8_t encr2wl_enc(int enc) {
    // the ESPHost returns authentication mode as encryption mode
    if (enc == WIFI_AUTH_OPEN) {
        return ENC_TYPE_NONE;
    } else if (enc == WIFI_AUTH_WEP) {
        return ENC_TYPE_WEP;
    } else if (enc == WIFI_AUTH_WPA_PSK) {
        return ENC_TYPE_WPA;
    } else if (enc == WIFI_AUTH_WPA2_PSK) {
        return ENC_TYPE_WPA2;
    } else if (enc == WIFI_AUTH_WPA_WPA2_PSK) {
        return ENC_TYPE_WPA2;
    } else if (enc == WIFI_AUTH_WPA3_PSK) {
        return ENC_TYPE_WPA3;
    } else if (enc == WIFI_AUTH_WPA2_WPA3_PSK) {
        return ENC_TYPE_WPA3;
    } else {
        return ENC_TYPE_UNKNOWN;
    }
}

uint8_t ESPHostLwIP::encryptionType() {
    return encr2wl_enc(ap.encryption_mode);
}

int8_t ESPHostLwIP::scanNetworks(bool async) {
    (void) async;
    accessPoints.clear();
    if (!initHW()) {
        return -1;
    }
    ethernet_arch_lwip_gpio_mask();
    int res = CEspControl::getInstance().getAccessPointScanList(accessPoints);
    ethernet_arch_lwip_gpio_unmask();
    wifiStatus = WL_SCAN_COMPLETED;
    if (res != ESP_CONTROL_OK) {
        return -1;
    }
    return accessPoints.size();
}

int8_t ESPHostLwIP::scanComplete() {
    return accessPoints.size();
}

void ESPHostLwIP::scanDelete() {
    accessPoints.clear();
}

const char* ESPHostLwIP::SSID(uint8_t networkItem) {
    if (networkItem < accessPoints.size()) {
        return (const char*) accessPoints[networkItem].ssid;
    }
    return nullptr;
}

uint8_t ESPHostLwIP::encryptionType(uint8_t networkItem) {
    if (networkItem < accessPoints.size()) {
        return encr2wl_enc(accessPoints[networkItem].encryption_mode);
    }
    return ENC_TYPE_UNKNOWN;
}

uint8_t* ESPHostLwIP::BSSID(uint8_t networkItem, uint8_t *bssid) {
    if (networkItem < accessPoints.size()) {
        CNetUtilities::macStr2macArray(bssid, (char*) accessPoints[networkItem].bssid);
    }
    return bssid;
}

uint8_t ESPHostLwIP::channel(uint8_t networkItem) {
    if (networkItem < accessPoints.size()) {
        return accessPoints[networkItem].channel;
    }
    return 0;
}

int32_t ESPHostLwIP::RSSI(uint8_t networkItem) {
    if (networkItem < accessPoints.size()) {
        return accessPoints[networkItem].rssi;
    }
    return 0;
}

void ESPHostLwIP::lowPowerMode() {
    if (!initHW()) {
        return;
    }
    ethernet_arch_lwip_gpio_mask();
    CEspControl::getInstance().setPowerSaveMode(1);
    ethernet_arch_lwip_gpio_unmask();
}

void ESPHostLwIP::noLowPowerMode() {
    // not supported by firmware
}
