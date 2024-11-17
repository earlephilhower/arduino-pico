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

#if defined(PICO_CYW43_SUPPORTED)

#include <lwip/netif.h>
extern "C" {
#include <cyw43.h>
#include <cyw43_stats.h>
}
#include <pico/cyw43_arch.h>
#include <pico/cyw43_driver.h>
#include <pico/lwip_nosys.h>
#include <hardware/resets.h>
#include <hardware/gpio.h>
#include <hardware/adc.h>
#include <hardware/clocks.h>
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

#ifndef WIFICC
#define WIFICC CYW43_COUNTRY_WORLDWIDE
#endif

// Taken from https://datasheets.raspberrypi.com/picow/connecting-to-the-internet-with-pico-w.pdf
// also discussion in https://github.com/earlephilhower/arduino-pico/issues/849
static bool CheckPicoW() {
#ifdef PICO_RP2040
    adc_init();
    auto dir = gpio_get_dir(29);
    auto fnc = gpio_get_function(29);
    adc_gpio_init(29);
    adc_select_input(3);
    auto adc29 = adc_read();
    gpio_set_function(29, fnc);
    gpio_set_dir(29, dir);

    dir = gpio_get_dir(25);
    fnc = gpio_get_function(25);
    gpio_init(25);
    gpio_set_dir(25, GPIO_IN);
    auto gp25 = gpio_get(25);
    gpio_set_function(25, fnc);
    gpio_set_dir(25, dir);

    if (gp25) {
        return true; // Can't tell, so assume yes
    } else if (adc29 < 200) {
        return true; // PicoW
    } else {
        return false;
    }
#else
    return true;
#endif
}

bool __isPicoW = true;

extern "C" void init_cyw43_wifi() {
    __isPicoW = CheckPicoW();
    if (__isPicoW) {
        // Fix for overclocked CPU: SPI communication breaks down with default "div by 2" speed
        // So, divide clock by 4 for anything including and above 250MHz CPU frequency.
        if (clock_get_hz(clk_sys) >= 250000000) {
            cyw43_set_pio_clock_divisor(4, 0); // div by 4.0
        }
        cyw43_arch_init_with_country(WIFICC);
    }
}

extern "C" void __lockBluetooth() {
    async_context_acquire_lock_blocking(cyw43_arch_async_context());
}

extern "C" void __unlockBluetooth() {
    async_context_release_lock(cyw43_arch_async_context());
}

extern "C" void __pinMode(pin_size_t pin, PinMode mode);
extern "C" void __digitalWrite(pin_size_t pin, PinStatus val);
extern "C" PinStatus __digitalRead(pin_size_t pin);

extern "C" void cyw43_pinMode(pin_size_t pin, PinMode mode) {
    if (!__isPicoW && (pin == PIN_LED)) {
        pin = 25;  // Silently swap in the Pico's LED
    }
    if (pin < 64) {
        __pinMode(pin, mode);
    } else {
        // TBD - There is no GPIO direction control in the driver
    }
}

extern "C" void cyw43_digitalWrite(pin_size_t pin, PinStatus val) {
    if (!__isPicoW && (pin == PIN_LED)) {
        pin = 25;  // Silently swap in the Pico's LED
    }
    if (pin < 64) {
        __digitalWrite(pin, val);
    } else {
        cyw43_arch_gpio_put(pin - 64, val == HIGH ? 1 : 0);
    }
}

extern "C" PinStatus cyw43_digitalRead(pin_size_t pin) {
    if (!__isPicoW && (pin == PIN_LED)) {
        pin = 25;  // Silently swap in the Pico's LED
    }
    if (pin < 64) {
        return __digitalRead(pin);
    } else {
        return cyw43_arch_gpio_get(pin - 64) ? HIGH : LOW;
    }
}

#endif
