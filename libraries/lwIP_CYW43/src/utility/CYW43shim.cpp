/*
    WiFi <-> LWIP driver for the CYG43 chip on the Raspberry Pico W

    Copyright (c) 2022 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include "CYW43shim.h"
extern "C" {
#include "cyw43.h"
#include "cyw43_stats.h"
}
#include "pico/cyw43_arch.h"
#include <Arduino.h>

// From cyw43_ctrl.c
#define WIFI_JOIN_STATE_KIND_MASK (0x000f)
#define WIFI_JOIN_STATE_ACTIVE  (0x0001)
#define WIFI_JOIN_STATE_FAIL    (0x0002)
#define WIFI_JOIN_STATE_NONET   (0x0003)
#define WIFI_JOIN_STATE_BADAUTH (0x0004)
#define WIFI_JOIN_STATE_AUTH    (0x0200)
#define WIFI_JOIN_STATE_LINK    (0x0400)
#define WIFI_JOIN_STATE_KEYED   (0x0800)
#define WIFI_JOIN_STATE_ALL     (0x0e01)


netif *CYW43::_netif = nullptr;

CYW43::CYW43(int8_t cs, arduino::SPIClass& spi, int8_t intrpin) {
    (void) cs;
    (void) spi;
    (void) intrpin;
    _netif = nullptr;
}

bool CYW43::begin(const uint8_t* address, netif* netif) {
    (void) address;
    _netif = netif;
    _self = &cyw43_state;

    if (!_ap) {
        _itf = 0;
        cyw43_arch_enable_sta_mode();
        cyw43_wifi_get_mac(_self, _itf, netif->hwaddr);

        auto authmode = CYW43_AUTH_WPA2_AES_PSK;
        if (_password == nullptr) {
            authmode = CYW43_AUTH_OPEN;
        }

        // Not currently possible to hook up igmp_mac_filter and mld_mac_filter
        // TODO: implement igmp_mac_filter and mld_mac_filter
        cyw43_set_allmulti(_self, true);

        if (cyw43_arch_wifi_connect_timeout_ms(_ssid, _password, authmode, _timeout)) {
            return false;
        } else {
            return true;
        }
    } else {
        _itf = 1;
        cyw43_arch_enable_ap_mode(_ssid, _password, _password ? CYW43_AUTH_WPA2_AES_PSK : CYW43_AUTH_OPEN);
        cyw43_wifi_get_mac(_self, _itf, netif->hwaddr);
        return true;
    }
}

void CYW43::end() {
    _netif = nullptr;
    cyw43_deinit(&cyw43_state);
}


uint16_t CYW43::sendFrame(const uint8_t* data, uint16_t datalen) {
    if (0 == cyw43_send_ethernet(_self, _itf, datalen, data, false)) {
        return datalen;
    }
    return 0;
}

uint16_t CYW43::readFrame(uint8_t* buffer, uint16_t bufsize) {
    // This is the polling method, but we hand this thru the interrupts
    (void) buffer;
    (void) bufsize;
    return 0;
}

// CB from the cyg32_driver
extern "C" void cyw43_cb_process_ethernet(void *cb_data, int itf, size_t len, const uint8_t *buf) {
    //cyw43_t *self = (cyw43_t *)cb_data
    (void) cb_data;
    (void) itf;
    struct netif *netif = CYW43::_netif; // &self->netif[itf];
#if CYW43_NETUTILS
    if (self->trace_flags) {
        cyw43_ethernet_trace(self, netif, len, buf, NETUTILS_TRACE_NEWLINE);
    }
#endif
    if (netif->flags & NETIF_FLAG_LINK_UP) {
        struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
        if (p != nullptr) {
            pbuf_take(p, buf, len);
            if (netif->input(p, netif) != ERR_OK) {
                pbuf_free(p);
            }
            CYW43_STAT_INC(PACKET_IN_COUNT);
        }
    }
}

extern "C" void cyw43_cb_tcpip_set_link_up(cyw43_t *self, int itf) {
    (void) self;
    (void) itf;
    if (CYW43::_netif) {
        netif_set_link_up(CYW43::_netif);
    }
}

extern "C" void cyw43_cb_tcpip_set_link_down(cyw43_t *self, int itf) {
    (void) self;
    (void) itf;
    if (CYW43::_netif) {
        netif_set_link_down(CYW43::_netif);
    }
    self->wifi_join_state &= ~WIFI_JOIN_STATE_ACTIVE;
}

extern "C" int cyw43_tcpip_link_status(cyw43_t *self, int itf) {
    //if ((CYW43::_netif->flags & (NETIF_FLAG_UP | NETIF_FLAG_LINK_UP)) == (NETIF_FLAG_UP | NETIF_FLAG_LINK_UP))
    //  Fake this since it's only used in the SDK
    if ((CYW43::_netif->flags & (NETIF_FLAG_LINK_UP)) == (NETIF_FLAG_LINK_UP)) {
        return CYW43_LINK_UP;
    } else {
        return cyw43_wifi_link_status(self, itf);
    }
}

// CBs from the SDK, not needed here as we do TCP later in the game
extern "C" void cyw43_cb_tcpip_init(cyw43_t *self, int itf) {
    (void) self;
    (void) itf;
}
extern "C" void cyw43_cb_tcpip_deinit(cyw43_t *self, int itf) {
    (void) self;
    (void) itf;
}

