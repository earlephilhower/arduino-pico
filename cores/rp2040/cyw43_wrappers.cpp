/*
    CYW43 TCP/Ethernet wrappers
    Copyright (c) 2023 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)

#include <lwip/netif.h>
extern "C" {
#include <cyw43.h>
#include <cyw43_stats.h>
}
#include <pico/cyw43_arch.h>
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

// The core can't directly call a library, so put in a dummy weak one to be overridden by one in lwip_cyw43 library
extern struct netif *__getCYW43Netif() __attribute__((weak));
struct netif *__getCYW43Netif() {
    return nullptr;
}

// CB from the cyw43 driver
extern "C" void __wrap_cyw43_cb_process_ethernet(void *cb_data, int itf, size_t len, const uint8_t *buf) {
    (void) cb_data;
    (void) itf;
    struct netif *netif = __getCYW43Netif();
    if (netif && (netif->flags & NETIF_FLAG_LINK_UP)) {
        struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
        if (p != nullptr) {
            pbuf_take(p, buf, len);
            if ((netif->input(p, netif) != ERR_OK)) {
                pbuf_free(p);
            }
            CYW43_STAT_INC(PACKET_IN_COUNT);
        }
    }
}

extern "C" void __wrap_cyw43_cb_tcpip_set_link_up(cyw43_t *self, int itf) {
    (void) self;
    (void) itf;
    struct netif *netif = __getCYW43Netif();
    if (netif) {
        netif_set_link_up(netif);
    }
}

extern "C" void __wrap_cyw43_cb_tcpip_set_link_down(cyw43_t *self, int itf) {
    (void) self;
    (void) itf;
    struct netif *netif = __getCYW43Netif();
    if (netif) {
        netif_set_link_down(netif);
    }
    self->wifi_join_state &= ~WIFI_JOIN_STATE_ACTIVE;
}

extern "C" int __wrap_cyw43_tcpip_link_status(cyw43_t *self, int itf) {
    struct netif *netif = __getCYW43Netif();
    //if ((CYW43::_netif->flags & (NETIF_FLAG_UP | NETIF_FLAG_LINK_UP)) == (NETIF_FLAG_UP | NETIF_FLAG_LINK_UP))
    //  Fake this since it's only used in the SDK
    if (netif && ((netif->flags & (NETIF_FLAG_LINK_UP)) == (NETIF_FLAG_LINK_UP))) {
        return CYW43_LINK_UP;
    } else {
        return cyw43_wifi_link_status(self, itf);
    }
}

// CBs from the SDK, not needed here as we do TCP later in the game
extern "C" void __wrap_cyw43_cb_tcpip_init(cyw43_t *self, int itf) {
    (void) self;
    (void) itf;
}
extern "C" void __wrap_cyw43_cb_tcpip_deinit(cyw43_t *self, int itf) {
    (void) self;
    (void) itf;
}

#endif
