/*
    FreeRTOS LWIP wrappers, implement a single LWIP work task

    Copyright (c) 2024 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include <functional>
#include <Arduino.h>
#include <pico/cyw43_arch.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <lwip/tcp.h>
#include <lwip/dns.h>
#include <lwip/raw.h>
#include <lwip/timeouts.h>
#include <lwip/apps/mdns.h>


extern void ethernet_arch_lwip_begin() __attribute__((weak));
extern void ethernet_arch_lwip_end() __attribute__((weak));
extern void ethernet_arch_lwip_gpio_mask() __attribute__((weak));
extern void ethernet_arch_lwip_gpio_unmask() __attribute__((weak));

//auto_init_recursive_mutex(__lwipMutex); // Only for case with no Ethernet or PicoW, but still doing LWIP (PPP?)
extern recursive_mutex_t __lwipMutex;

// When we have a GPIO IRQ for packet reception, the IntfDev will check if we're already doing lwip.
// If so, it'll disable the IRQ and flag that we need to re-enable it as soon as the current LWIP call ends
extern volatile int __inLWIP;
extern volatile bool __needsIRQEN;

// LWIPMutex is a no-op under FreeRTOS because we wrap all calls and just send a request message to
// the LWIP task.  No locking needed, many people can call LWIP in parallel but the messages will be
// processed 1-threaded in order or reception

// Under Non-OS mode, we do need to lock the context because the threadsafe IRQ could come in

class LWIPMutex {
public:
    LWIPMutex() {
#if !defined(__FREERTOS)
        __inLWIP = __inLWIP + 1;
        if (ethernet_arch_lwip_begin) {
            ethernet_arch_lwip_begin();
        } else {
            recursive_mutex_enter_blocking(&__lwipMutex);
        }
#endif
    }

    ~LWIPMutex() {
#if !defined(__FREERTOS)
        if (ethernet_arch_lwip_end) {
            ethernet_arch_lwip_end();
        } else {
            recursive_mutex_exit(&__lwipMutex);
        }
        __inLWIP = __inLWIP - 1;
        if (__needsIRQEN && !__inLWIP) {
            __needsIRQEN = false;
            ethernet_arch_lwip_gpio_unmask();
        }
#endif
    }
};



#ifdef __cplusplus
extern "C" {
#endif

// Implement all LWIP operations in a single FreeRTOS task.  Calls to LWIP will
// actually post work on the work queue and wait until the LWIP task indicates
// completion

// Enumerated type for LWIP request
typedef enum {
    __lwip_init = 1000,

#include "./lwip-wrappers/wrap_pbuf_enums.inc"
#include "./lwip-wrappers/wrap_tcp_enums.inc"
#include "./lwip-wrappers/wrap_udp_enums.inc"
#include "./lwip-wrappers/wrap_raw_enums.inc"
#include "./lwip-wrappers/wrap_netif_enums.inc"
#include "./lwip-wrappers/wrap_dns_enums.inc"
#include "./lwip-wrappers/wrap_dhcp_enums.inc"
#include "./lwip-wrappers/wrap_igmp_enums.inc"
#include "./lwip-wrappers/wrap_mld6_enums.inc"
#include "./lwip-wrappers/wrap_mdns_enums.inc"
#include "./lwip-wrappers/wrap_sntp_enums.inc"

    // Manually implemented, they're special
    __sys_check_timeouts,
    __sys_timeouts_sleeptime,

    __ethernet_input,

#if defined(PICO_CYW43_SUPPORTED)
    __cyw43_wifi_join,
    __cyw43_wifi_leave,
    __cyw43_ioctl,
    __cyw43_wifi_update_multicast_filter,
#endif

    __callback,
    __function,
} __lwip_op;

// Set up a local request buffer and call this to add to lwip work queue.  Will only return once lwip operation completed
// LWIP callbacks will happen from the LWIP task at some future time

extern void __real_lwip_init();

#include "./lwip-wrappers/wrap_pbuf_externs.inc"
#include "./lwip-wrappers/wrap_tcp_externs.inc"
#include "./lwip-wrappers/wrap_udp_externs.inc"
#include "./lwip-wrappers/wrap_raw_externs.inc"
#include "./lwip-wrappers/wrap_netif_externs.inc"
#include "./lwip-wrappers/wrap_dns_externs.inc"
#include "./lwip-wrappers/wrap_dhcp_externs.inc"
#include "./lwip-wrappers/wrap_igmp_externs.inc"
#include "./lwip-wrappers/wrap_mld6_externs.inc"
#include "./lwip-wrappers/wrap_mdns_externs.inc"
#include "./lwip-wrappers/wrap_sntp_externs.inc"

extern void __real_sys_check_timeouts();
extern u32_t __real_sys_timeouts_sleeptime();
extern err_t __real_ethernet_input(struct pbuf *p, struct netif *netif);

extern int __real_cyw43_wifi_join(cyw43_t *self, size_t ssid_len, const uint8_t *ssid, size_t key_len, const uint8_t *key, uint32_t auth_type, const uint8_t *bssid, uint32_t channel);
extern int __real_cyw43_wifi_leave(cyw43_t *self, int itf);
extern int __real_cyw43_ioctl(cyw43_t *self, uint32_t cmd, size_t len, uint8_t *buf, uint32_t iface);
extern int __real_cyw43_wifi_update_multicast_filter(cyw43_t *self, uint8_t *addr, bool add);
extern bool __real_cyw43_driver_init(async_context_t *context);
extern void __real_cyw43_driver_deinit(async_context_t *context);
extern void __real_cyw43_thread_enter();
extern void __real_cyw43_thread_exit();
extern void __real_cyw43_thread_lock_check();
extern void __real_cyw43_await_background_or_timeout_us(uint32_t timeout_us);
extern void __real_cyw43_delay_ms(uint32_t ms);
extern void __real_cyw43_delay_us(uint32_t us);
extern void __real_cyw43_post_poll_hook();
extern void __real_cyw43_await_background_or_timeout_us(uint32_t timeout_us);
extern void __real_cyw43_schedule_internal_poll_dispatch(void (*func)());
extern void __real_cyw43_arch_gpio_put(uint wl_gpio, bool value);

#include "./lwip-wrappers/wrap_pbuf_structs.inc"
#include "./lwip-wrappers/wrap_tcp_structs.inc"
#include "./lwip-wrappers/wrap_udp_structs.inc"
#include "./lwip-wrappers/wrap_raw_structs.inc"
#include "./lwip-wrappers/wrap_netif_structs.inc"
#include "./lwip-wrappers/wrap_dns_structs.inc"
#include "./lwip-wrappers/wrap_dhcp_structs.inc"
#include "./lwip-wrappers/wrap_igmp_structs.inc"
#include "./lwip-wrappers/wrap_mld6_structs.inc"
#include "./lwip-wrappers/wrap_mdns_structs.inc"
#include "./lwip-wrappers/wrap_sntp_structs.inc"

typedef struct {
    u32_t *ret;
} __sys_timeouts_sleeptime_req;

typedef struct {
    struct pbuf *p;
    struct netif *netif;
    err_t *ret;
} __ethernet_input_req;

#if defined(PICO_CYW43_SUPPORTED)
typedef struct {
    cyw43_t *self;
    size_t ssid_len;
    const uint8_t *ssid;
    size_t key_len;
    const uint8_t *key;
    uint32_t auth_type;
    const uint8_t *bssid;
    uint32_t channel;
    int *ret;
} __cyw43_wifi_join_req;

typedef struct {
    cyw43_t *self;
    int itf;
    int *ret;
} __cyw43_wifi_leave_req;

typedef struct {
    cyw43_t *self;
    uint32_t cmd;
    size_t len;
    uint8_t *buf;
    uint32_t iface;
    int *ret;
} __cyw43_ioctl_req;

typedef struct {
    cyw43_t *self;
    uint8_t *addr;
    bool add;
    int *ret;
} __cyw43_wifi_update_multicast_filter_req;
#endif

// Run a callback in the LWIP thread (i.e. for Ethernet device polling and packet reception)
// When in an interrupt, need to pass in a heap-allocated buffer
typedef struct {
    void (*cb)(void *);
    void *cbData;
} __callback_req;
extern void lwip_callback(void (*cb)(void *), void *cbData, __callback_req *buffer = nullptr);

#ifdef __cplusplus
};

extern void lwip_callback(std::function<void(void)> cb);

#endif
