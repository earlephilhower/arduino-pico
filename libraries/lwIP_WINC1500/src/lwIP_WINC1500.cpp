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

#include "lwIP_WINC1500.h"

extern "C" {
#include "driver/include/m2m_periph.h"
}
#include <lwip/netif.h>

WINC1500LwIP *WINC1500LwIP::instance = nullptr;

static uint8_t rx_buf[1500];

WINC1500LwIP::WINC1500LwIP() :
        LwipIntfDev<WINC1500>() {

}

void WINC1500LwIP::setSTA() {
    apMode = false;
}

void WINC1500LwIP::setAP() {
    apMode = true;
}

void WINC1500LwIP::setSSID(const char *ssid) {
    if (apMode) {
        if (ssid == nullptr) {
            _apConfig.au8SSID[0] = 0;
        } else {
            memcpy(_apConfig.au8SSID, ssid, sizeof(_apConfig.au8SSID));
        }
    } else {
        if (ssid == nullptr) {
            _connInfo.acSSID[0] = 0;
        } else {
            memcpy(_connInfo.acSSID, ssid, sizeof(_connInfo.acSSID));
        }
    }
}

void WINC1500LwIP::setBSSID(const uint8_t *bssid) {
    (void) bssid;
    // connecting with BSSID is not supported with ATWINC
}

void WINC1500LwIP::setPassword(const char *pass) {
    if (apMode) {
        if (pass == nullptr) {
            _apConfig.au8Key[0] = 0;
            _apConfig.u8KeySz = 0;
        } else {
            memcpy(_apConfig.au8Key, pass, sizeof(_apConfig.au8Key));
            _apConfig.u8KeySz = strlen(pass);
        }
    } else {
        if (pass == nullptr) {
            _pwd[0] = 0;
        } else {
            memcpy(_pwd, pass, sizeof(_pwd));
        }
    }
}

void WINC1500LwIP::setChannel(uint8_t channel) {
    tenuM2mScanCh ch =  (tenuM2mScanCh) channel;
    ch = (ch < M2M_WIFI_CH_1 || ch > M2M_WIFI_CH_ALL) ? M2M_WIFI_CH_ALL : ch;
    if (apMode) {
        _apConfig.u8ListenChannel = ch;
    } else {
        _channel = ch;
    }
}

void WINC1500LwIP::setTimeout(int timeout) {
    _timeout = timeout;
}

static void wifi_cb(uint8_t u8MsgType, void *pvMsg) {
    WINC1500LwIP::instance->handleEvent(u8MsgType, pvMsg);
}

void WINC1500LwIP::handleEvent(uint8_t u8MsgType, void *pvMsg) {
    switch (u8MsgType) {
    case M2M_WIFI_RESP_DEFAULT_CONNECT: {
        tstrM2MDefaultConnResp *pstrDefaultConnResp = (tstrM2MDefaultConnResp*) pvMsg;
        if (pstrDefaultConnResp->s8ErrorCode) {
            _connInfo.acSSID[0] = 0;
            _status = WL_DISCONNECTED;
        }
        break;
    }
    case M2M_WIFI_RESP_CON_STATE_CHANGED: {
        tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged*) pvMsg;
        if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
            //SERIAL_PORT_MONITOR.println("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: CONNECTED");
            if (!apMode) {
                joined = true;
                _connInfo.acSSID[0] = 0;
                _status = WL_CONNECTED;
#ifdef CONF_PERIPH
                // WiFi led ON.
                m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 0);
#endif
            } else {
                _status = WL_AP_LISTENING;
            }
        } else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
            //SERIAL_PORT_MONITOR.println("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: DISCONNECTED");
            if (!apMode) {
                LwipIntfDev<WINC1500>::end();
                joined = false;
                _connInfo.acSSID[0] = 0;
                _status = WL_CONNECTION_LOST;
            } else {
                _status = WL_AP_LISTENING;
            }
#ifdef CONF_PERIPH
            // WiFi led OFF (rev A then rev B).
            m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 1);
            m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 1);
#endif
        }
        break;
    }
    case M2M_WIFI_RESP_CURRENT_RSSI: {
        _resolve = *((int8_t*) pvMsg);
        break;
    }
    case M2M_WIFI_RESP_PROVISION_INFO: {
        tstrM2MProvisionInfo *pstrProvInfo = (tstrM2MProvisionInfo*) pvMsg;
        //SERIAL_PORT_MONITOR.println("wifi_cb: M2M_WIFI_RESP_PROVISION_INFO");
        _connInfo.acSSID[0] = 0;

        if (pstrProvInfo->u8Status == M2M_SUCCESS) {
            m2m_wifi_connect((char*) pstrProvInfo->au8SSID, strlen((char*) pstrProvInfo->au8SSID), //
                    pstrProvInfo->u8SecType, pstrProvInfo->au8Password, M2M_WIFI_CH_ALL);
        } else {
//            _status = WL_PROVISIONING_FAILED;
//            SERIAL_PORT_MONITOR.println("wifi_cb: Provision failed.\r\n");
//            beginProvision();
        }
        break;
    }
    case M2M_WIFI_RESP_SCAN_DONE: {
//        tstrM2mScanDone *pstrInfo = (tstrM2mScanDone*) pvMsg;
        _status = WL_SCAN_COMPLETED;
        break;
    }
    case M2M_WIFI_RESP_SCAN_RESULT: {
        tstrM2mWifiscanResult *pstrScanResult = (tstrM2mWifiscanResult*) pvMsg;
        _scanResult = *pstrScanResult;
        _scanResult.u8index++;
        break;
    }
    case M2M_WIFI_RESP_CONN_INFO: {
        tstrM2MConnInfo *pstrConnInfo = (tstrM2MConnInfo*) pvMsg;
        _connInfo = *pstrConnInfo;
        break;
    }
    default:
        break;
    }
}

static void winc_netif_rx_callback(uint8 u8MsgType, void *pvMsg, void *pvCtrlBuf) {
    switch (u8MsgType) {
    case M2M_WIFI_RESP_ETHERNET_RX_PACKET: {
        tstrM2mIpCtrlBuf* ipCtrlBuf = (tstrM2mIpCtrlBuf*) pvCtrlBuf;
        uint16_t len = ipCtrlBuf->u16DataSize;
        struct netif *netif = WINC1500LwIP::instance->getNetIf();
        if (netif && (netif->flags & NETIF_FLAG_LINK_UP)) {
            struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
            if (p != nullptr) {
                pbuf_take(p, pvMsg, len);
                ethernet_arch_lwip_gpio_mask();
                if (netif->input(p, netif) != ERR_OK) {
                    pbuf_free(p);
                }
                ethernet_arch_lwip_gpio_unmask();
            }
        }
    }
        break;
    default:
        break;
    }
}

bool WINC1500LwIP::initHW() {
    if (wifiHwInitialized) {
        return true;
    }
    nm_bsp_init();

    instance = this;
    tstrWifiInitParam param;
    param.pfAppWifiCb = wifi_cb;

    // 'ETH' mode https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ApplicationNotes/ApplicationNotes/70005333A.pdf
    param.strEthInitParam.pfAppEthCb = winc_netif_rx_callback;
    param.strEthInitParam.au8ethRcvBuf = rx_buf;
    param.strEthInitParam.u16ethRcvBufSize = sizeof(rx_buf);
    param.strEthInitParam.u8EthernetEnable = M2M_WIFI_MODE_ETHERNET; //For bypassing the TCPIP Stack of WINC

    int8_t ret = m2m_wifi_init(&param);
    if (M2M_SUCCESS != ret && M2M_ERR_FW_VER_MISMATCH != ret) {
#ifdef CONF_PERIPH
        if (ret != M2M_ERR_INVALID) {
            // Error led ON (rev A then rev B).
            m2m_periph_gpio_set_val(M2M_PERIPH_GPIO18, 0);
            m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO6, 1);
        }
#endif
        return false;
    }

#ifdef CONF_PERIPH
    // Initialize IO expander LED control (rev A then rev B)..
    m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 1);
    m2m_periph_gpio_set_val(M2M_PERIPH_GPIO16, 1);
    m2m_periph_gpio_set_val(M2M_PERIPH_GPIO18, 1);
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO15, 1);
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO16, 1);
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO18, 1);
    m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 1);
    m2m_periph_gpio_set_val(M2M_PERIPH_GPIO5, 1);
    m2m_periph_gpio_set_val(M2M_PERIPH_GPIO6, 1);
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO4, 1);
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO5, 1);
    m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO6, 1);
#endif

    wifiHwInitialized = true;
    _status = WL_IDLE_STATUS;
    __startEthernetContext();
    return true;
}

bool WINC1500LwIP::begin() {
    if (!initHW()) {
        return false;
    }
    ethernet_arch_lwip_gpio_mask();
    if (!apMode) {
        uint8 secType = M2M_WIFI_SEC_WPA_PSK;
        if (_pwd[0] == 0) {
            secType = M2M_WIFI_SEC_OPEN;
        }
        if (m2m_wifi_connect(_connInfo.acSSID, strlen(_connInfo.acSSID), secType, (void*) _pwd, _channel) < 0) {
            _status = WL_CONNECT_FAILED;
            ethernet_arch_lwip_end();
            return false;
        }
        _status = WL_IDLE_STATUS;
        for (unsigned long start = millis(); millis() - start < _timeout;) {
            m2m_wifi_handle_events(NULL);
            if ((_status & WL_CONNECTED) || (_status & WL_DISCONNECTED)) {
                break;
            }
        }
        if (_status != WL_CONNECTED) {
            _status = WL_CONNECT_FAILED;
            ethernet_arch_lwip_end();
            return false;
        }
    } else {
        _apConfig.u8SecType = _apConfig.au8Key[0] == 0 ? M2M_WIFI_SEC_OPEN : M2M_WIFI_SEC_WPA_PSK;
        if (_apConfig.u8ListenChannel == 0) {
            _apConfig.u8ListenChannel = M2M_WIFI_CH_1;
        }
        _apConfig.au8DHCPServerIP[0] = 192; // not used but checked
        if (m2m_wifi_enable_ap(&_apConfig) < 0) {
            _status = WL_AP_FAILED;
            return false;
        }
#ifdef CONF_PERIPH
        // WiFi led ON (rev A then rev B).
        m2m_periph_gpio_set_val(M2M_PERIPH_GPIO15, 0);
        m2m_periph_gpio_set_val(M2M_PERIPH_GPIO4, 0);
#endif
    }
    ethernet_arch_lwip_gpio_unmask();
    uint8_t mac[6];
    if (!LwipIntfDev<WINC1500>::begin(macAddress(apMode, mac))) {
        ethernet_arch_lwip_gpio_mask();
        if (apMode) {
            m2m_wifi_disable_ap();
            _status = WL_AP_FAILED;
        } else {
            m2m_wifi_disconnect();
            _status = WL_CONNECT_FAILED;
        }
        ethernet_arch_lwip_gpio_unmask();
        return false;
    }
    if (apMode) {
        _status = WL_AP_LISTENING;
    } else {
        _status = LwipIntfDev<WINC1500>::connected() ? WL_CONNECTED : WL_DISCONNECTED;
    }
    return true;
}

void WINC1500LwIP::end() {
    if (apMode) {
        m2m_wifi_disable_ap();
        LwipIntfDev<WINC1500>::end();
    } else {
        m2m_wifi_disconnect();
        unsigned long start = millis();
        while (joined && millis() - start < 5000) {
            m2m_wifi_handle_events(NULL);
        }
    }
}

bool WINC1500LwIP::connected() {
    return (status() == WL_CONNECTED);
}

uint8_t WINC1500LwIP::status() {
    initHW();
    if (joined && _status == WL_DISCONNECTED && LwipIntfDev<WINC1500>::connected()) {
        _status = WL_CONNECTED; // DHCP finished
    }
    return _status;
}

uint8_t* WINC1500LwIP::macAddress(bool apMode, uint8_t *mac) {
    (void) apMode;
    if (!initHW()) {
        return mac;
    }
    m2m_wifi_get_mac_address(mac);
    return mac;
}

bool WINC1500LwIP::requestConnInfo() {
    if (!joined) {
        return false;
    }
    if (_connInfo.acSSID[0] == 0) {
        m2m_wifi_get_connection_info();
        unsigned long start = millis();
        while (_connInfo.acSSID[0] == 0 && millis() - start < 1000) {
            m2m_wifi_handle_events(NULL);
        }
    }
    return _connInfo.acSSID[0] != 0;
}

uint8_t* WINC1500LwIP::BSSID(uint8_t *bssid) {
    if (requestConnInfo()) {
        memcpy(bssid, _connInfo.au8MACAddress, 6);
    }
    return bssid;
}

int WINC1500LwIP::channel() {
//    if (requestConnInfo()) {
//        return _connInfo.u8CurrChannel;
//    }
    return _channel;
}

int32_t WINC1500LwIP::RSSI() {
    if (!joined) {
        return 0;
    }
    _resolve = 0;
    if (m2m_wifi_req_curr_rssi() < 0) {
        return 0;
    }

    // Wait for connection or timeout:
    unsigned long start = millis();
    while (_resolve == 0 && millis() - start < 1000) {
        m2m_wifi_handle_events(NULL);
    }

    int32_t rssi = _resolve;

    _resolve = 0;

    return rssi;
}

static uint8_t authtype2wl_enc(int enc) {
    switch (enc) {
    case M2M_WIFI_SEC_OPEN:
        return ENC_TYPE_NONE;
    case M2M_WIFI_SEC_WPA_PSK:
        return ENC_TYPE_WPA;
    case M2M_WIFI_SEC_WEP:
        return ENC_TYPE_WEP;
    case M2M_WIFI_SEC_802_1X:
        return ENC_TYPE_UNKNOWN;
    default:
        return ENC_TYPE_UNKNOWN;
    }
}

uint8_t WINC1500LwIP::encryptionType() {
    if (requestConnInfo()) {
        return authtype2wl_enc(_connInfo.u8SecType);
    }
    return ENC_TYPE_UNKNOWN;
}

int8_t WINC1500LwIP::scanNetworks(bool async) {
    _scanResult.u8index = 0;
    if (!initHW()) {
        return -1;
    }
    if (m2m_wifi_request_scan(M2M_WIFI_CH_ALL) < 0) {
        return 0;
    }
    if (async) {
        return 0;
    }

    // Wait for scan result or timeout:
    _status = WL_IDLE_STATUS;
    unsigned long start = millis();
    while (!(_status & WL_SCAN_COMPLETED) && millis() - start < 5000) {
        m2m_wifi_handle_events(NULL);
    }
    return m2m_wifi_get_num_ap_found();
}

int8_t WINC1500LwIP::scanComplete() {
    return _status = WL_SCAN_COMPLETED;
}

void WINC1500LwIP::scanDelete() {
    _scanResult.u8index = 0;
}

bool WINC1500LwIP::requestScanResult(uint8_t networkItem) {
    if (_scanResult.u8index - 1 != networkItem) {
        if (m2m_wifi_req_scan_result(networkItem) < 0) {
            return false;
        }
        unsigned long start = millis();
        while (_scanResult.u8index - 1 != networkItem && millis() - start < 2000) {
            m2m_wifi_handle_events(NULL);
        }
    }
    return (_scanResult.u8index - 1 == networkItem);
}

const char* WINC1500LwIP::SSID(uint8_t networkItem) {
    if (!requestScanResult(networkItem))
        return nullptr;
    return (char*) _scanResult.au8SSID;
}

uint8_t WINC1500LwIP::encryptionType(uint8_t networkItem) {
    if (!requestScanResult(networkItem))
        return ENC_TYPE_UNKNOWN;
    return authtype2wl_enc(_scanResult.u8AuthType);
}

uint8_t* WINC1500LwIP::BSSID(uint8_t networkItem, uint8_t *bssid) {
    if (!requestScanResult(networkItem))
        return bssid;
    memcpy(bssid, _scanResult.au8BSSID, 6);
    return bssid;
}

uint8_t WINC1500LwIP::channel(uint8_t networkItem) {
    if (!requestScanResult(networkItem))
        return 0;
    return _scanResult.u8ch;
}

int32_t WINC1500LwIP::RSSI(uint8_t networkItem) {
    if (!requestScanResult(networkItem))
        return 0;
    return _scanResult.s8rssi;
}

void WINC1500LwIP::lowPowerMode() {
    if (!initHW()) {
        return;
    }
    m2m_wifi_set_sleep_mode(M2M_PS_H_AUTOMATIC, true);
}

void WINC1500LwIP::noLowPowerMode() {
    if (!initHW()) {
        return;
    }
    m2m_wifi_set_sleep_mode(M2M_NO_PS, false);
}
