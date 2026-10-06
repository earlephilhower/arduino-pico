/*
    LWIP wrappers to protect against timer-based re-entrancy

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

#include <functional>
#include <Arduino.h>
#include <pico/mutex.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <lwip/tcp.h>
#include <lwip/dns.h>
#include <lwip/raw.h>
#include <lwip/timeouts.h>
#include <pico/cyw43_arch.h>
#include <pico/mutex.h>
#include <sys/lock.h>
#include "_xoshiro.h"
#include "lwip_wrap.h"
#include <pico/btstack_run_loop_async_context.h>
#ifdef __FREERTOS
#include "freertos/freertos-lwip.h"
#endif

//auto_init_recursive_mutex(__lwipMutex); // Only for case with no Ethernet or PicoW, but still doing LWIP (PPP?)
recursive_mutex_t __lwipMutex;

// When we have a GPIO IRQ for packet reception, the IntfDev will check if we're already doing lwip.
// If so, it'll disable the IRQ and flag that we need to re-enable it as soon as the current LWIP call ends
volatile int __inLWIP = 0;
volatile bool __needsIRQEN = false;

extern "C" {

    extern void __lwip(__lwip_op op, void *req, bool fromISR = false);
    extern bool __isLWIPThread();
    extern async_context_t *__getEthernetContext() __attribute__((weak));;

    void __lwip_assert_core_locked() {
#ifdef __FREERTOS
        if (!__isLWIPThread()) {
            panic("LWIP_ASSERT_CORE_LOCKED failed");
        }
#else
        if (__getEthernetContext) {
            async_context_lock_check(__getEthernetContext());
        }
#endif
    }

    static XoshiroCpp::Xoshiro256PlusPlus *_lwip_rng = nullptr;
    // Random number generator for LWIP
    unsigned long __lwip_rand() {
        return (unsigned long)(*_lwip_rng)();
    }

    // Avoid calling lwip_init multiple times
    void __wrap_lwip_init() {
        if (!_lwip_rng) {
            recursive_mutex_init(&__lwipMutex);
            _lwip_rng = new XoshiroCpp::Xoshiro256PlusPlus(micros());
            {
                LWIPMutex m;
                __real_lwip_init();
            }
#ifdef __FREERTOS
            __startLWIPThread();
#endif
        }
    }

#include "./lwip-wrappers/wrap_pbuf_functions.inc"
#include "./lwip-wrappers/wrap_raw_functions.inc"
#include "./lwip-wrappers/wrap_tcp_functions.inc"
#include "./lwip-wrappers/wrap_udp_functions.inc"
#include "./lwip-wrappers/wrap_netif_functions.inc"
#include "./lwip-wrappers/wrap_dns_functions.inc"
#include "./lwip-wrappers/wrap_dhcp_functions.inc"
#include "./lwip-wrappers/wrap_igmp_functions.inc"
#include "./lwip-wrappers/wrap_mld6_functions.inc"
#include "./lwip-wrappers/wrap_mdns_functions.inc"
#include "./lwip-wrappers/wrap_sntp_functions.inc"
#include "./lwip-wrappers/wrap_ethernet_functions.inc"

    // sys_check_timeouts is special case because the async process will call it.  If we're already in a timeout check, just do a noop
    void __wrap_sys_check_timeouts(void) {
#ifdef __FREERTOS
        if (!__isLWIPThread()) {
            __lwip(__sys_check_timeouts, nullptr);
            return;
        }
#endif
        LWIPMutex m;
        __real_sys_check_timeouts();
    }

    u32_t __wrap_sys_timeouts_sleeptime() {
#ifdef __FREERTOS
        if (!__isLWIPThread()) {
            u32_t ret;
            __sys_timeouts_sleeptime_req req = { &ret };
            __lwip(__sys_timeouts_sleeptime, &req);
            return ret;
        }
#endif
        LWIPMutex m;
        return __real_sys_timeouts_sleeptime();
    }


#if defined(PICO_CYW43_SUPPORTED)
    int __real_cyw43_wifi_join(cyw43_t *self, size_t ssid_len, const uint8_t *ssid, size_t key_len, const uint8_t *key, uint32_t auth_type, const uint8_t *bssid, uint32_t channel);
    int __wrap_cyw43_wifi_join(cyw43_t *self, size_t ssid_len, const uint8_t *ssid, size_t key_len, const uint8_t *key, uint32_t auth_type, const uint8_t *bssid, uint32_t channel) {
#ifdef __FREERTOS
        if (!__isLWIPThread()) {
            int ret;
            __cyw43_wifi_join_req req = { self, ssid_len, ssid, key_len, key, auth_type, bssid, channel, &ret };
            __lwip(__cyw43_wifi_join, &req);
            return ret;
        }
#endif
        return __real_cyw43_wifi_join(self, ssid_len, ssid, key_len, key, auth_type, bssid, channel);
    }

    int __real_cyw43_wifi_leave(cyw43_t* self, int itf);
    int __wrap_cyw43_wifi_leave(cyw43_t* self, int itf) {
#ifdef __FREERTOS
        if (!__isLWIPThread()) {
            int ret;
            __cyw43_wifi_leave_req req = { self, itf, &ret };
            __lwip(__cyw43_wifi_leave, &req);
            return ret;
        }
#endif
        return __real_cyw43_wifi_leave(self, itf);
    }

    int __real_cyw43_ioctl(cyw43_t *self, uint32_t cmd, size_t len, uint8_t *buf, uint32_t iface);
    int __wrap_cyw43_ioctl(cyw43_t *self, uint32_t cmd, size_t len, uint8_t *buf, uint32_t iface) {
#ifdef __FREERTOS
        if (!__isLWIPThread()) {
            int ret;
            __cyw43_ioctl_req req = { self, cmd, len, buf, iface, &ret };
            __lwip(__cyw43_ioctl, &req);
            return ret;
        }
#endif
        return __real_cyw43_ioctl(self, cmd, len, buf, iface);
    }

    int __real_cyw43_wifi_update_multicast_filter(cyw43_t *self, uint8_t *addr, bool add);
    int __wrap_cyw43_wifi_update_multicast_filter(cyw43_t *self, uint8_t *addr, bool add) {
#ifdef __FREERTOS
        if (!__isLWIPThread()) {
            int ret;
            __cyw43_wifi_update_multicast_filter_req req = { self, addr, add, &ret };
            __lwip(__cyw43_wifi_update_multicast_filter, &req);
            return ret;
        }
#endif
        return __real_cyw43_wifi_update_multicast_filter(self, addr, add);
    }
#endif


    void lwip_callback(void (*cb)(void *), void *cbData, __callback_req *buffer) {
#ifdef __FREERTOS
        if (buffer) {
            buffer->cb = cb;
            buffer->cbData = cbData;
            __lwip(__callback, buffer, true);
            return;
        } else if (!__isLWIPThread()) {
            __callback_req req = { cb, cbData };
            __lwip(__callback, &req, false);
            return;
        }
#endif
        (void) buffer;
        cb(cbData);
        return;
    }

#ifndef __FREERTOS
    bool __wrap_cyw43_driver_init(async_context_t *context) {
        return __real_cyw43_driver_init(context);
    }
    void __wrap_cyw43_driver_deinit(async_context_t *context) {
        __real_cyw43_driver_deinit(context);
    }
    void __wrap_cyw43_thread_enter() {
        __real_cyw43_thread_enter();
    }
    void __wrap_cyw43_thread_exit() {
        __real_cyw43_thread_exit();
    }
    void __wrap_cyw43_thread_lock_check() {
        __real_cyw43_thread_lock_check();
    }
    void _wrap_cyw43_await_background_or_timeout_us(uint32_t timeout_us) {
        __real_cyw43_await_background_or_timeout_us(timeout_us);
    }
    void __wrap_cyw43_delay_ms(uint32_t ms) {
        __real_cyw43_delay_ms(ms);
    }
    void __wrap_cyw43_delay_us(uint32_t us) {
        __real_cyw43_delay_us(us);
    }
    void __wrap_cyw43_post_poll_hook() {
        __real_cyw43_post_poll_hook();
    }
    void __wrap_cyw43_await_background_or_timeout_us(uint32_t timeout_us) {
        __real_cyw43_await_background_or_timeout_us(timeout_us);
    }
    void __wrap_cyw43_schedule_internal_poll_dispatch(void (*func)()) {
        __real_cyw43_schedule_internal_poll_dispatch(func);
    }
    void __wrap_cyw43_arch_gpio_put(uint wl_gpio, bool value) {
        __real_cyw43_arch_gpio_put(wl_gpio, value);
    }

    extern const btstack_run_loop_t *__real_btstack_run_loop_async_context_get_instance(async_context_t *async_context);
    const btstack_run_loop_t *__wrap_btstack_run_loop_async_context_get_instance(async_context_t *async_context) {
        return __real_btstack_run_loop_async_context_get_instance(async_context);
    }


#endif

}; // extern "C"

void lwip_callback(std::function<void(void)> cb) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __lwip(&cb);
        return;
    }
#endif
    cb();
}
