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

#include <Arduino.h>
#include <pico/cyw43_arch.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <lwip/tcp.h>
#include <lwip/dns.h>
#include <lwip/raw.h>
#include <lwip/timeouts.h>


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
        __inLWIP++;
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
        __inLWIP--;
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

    __pbuf_header = 2000,
    __pbuf_free,
    __pbuf_alloc,
    __pbuf_take,
    __pbuf_copy_partial,
    __pbuf_ref,
    __pbuf_get_at,
    __pbuf_get_contiguous,
    __pbuf_cat,

    __tcp_backlog_delayed = 3000,
    __tcp_backlog_accepted,
    __tcp_close,
    __tcp_shutdown,
    __tcp_abort,
    __tcp_bind,
    __tcp_bind_netif,
    __tcp_listen_with_backlog,
    //    __tcp_listen_with_backlog_and_err,
    __tcp_recved,
    __tcp_connect,
    __tcp_new,
    __tcp_new_ip_type,
    __tcp_arg,
    __tcp_recv,
    __tcp_sent,
    __tcp_err,
    __tcp_accept,
    __tcp_poll,
    __tcp_write,
    __tcp_output,

    __tcp_setprio,

    __udp_send = 4000,
    __udp_sendto,
    __udp_sendto_if,
    __udp_sendto_if_src,
    __udp_bind,
    __udp_connect,
    __udp_disconnect,
    __udp_recv,
    __udp_remove,
    __udp_new,
    __udp_new_ip_type,

    __sys_check_timeouts = 5000,

    __dns_gethostbyname = 6000,
    __dns_gethostbyname_addrtype,

    __raw_bind = 7000,
    __raw_connect,
    __raw_recv,
    __raw_sendto,
    __raw_send,
    __raw_remove,
    __raw_new,
    __raw_new_ip_type,

    __netif_add = 8000,
    __netif_remove,

    __ethernet_input = 9000,

#if defined(PICO_CYW43_SUPPORTED)
    __cyw43_wifi_join = 9500,
    __cyw43_wifi_leave,
#endif

    __callback = 10000,
} __lwip_op;

// Set up a local request buffer and call this to add to lwip work queue.  Will only return once lwip operation completed
// LWIP callbacks will happen from the LWIP task at some future time

extern void __real_lwip_init();
extern u8_t __real_pbuf_header(struct pbuf *p, s16_t header_size);
extern u8_t __real_pbuf_free(struct pbuf *p);
extern struct pbuf *__real_pbuf_alloc(pbuf_layer l, u16_t length, pbuf_type type);
extern err_t __real_pbuf_take(struct pbuf *buf, const void *dataptr, u16_t len);
extern u16_t __real_pbuf_copy_partial(const struct pbuf *p, void *dataptr, u16_t len, u16_t offset);
extern void __real_pbuf_ref(struct pbuf *p);
extern u8_t __real_pbuf_get_at(const struct pbuf* p, u16_t offset);
extern void *__real_pbuf_get_contiguous(const struct pbuf *p, void *buffer, size_t bufsize, u16_t len, u16_t offset);
extern void __real_pbuf_cat(struct pbuf *head, struct pbuf *tail);
extern void __real_tcp_arg(struct tcp_pcb *pcb, void *arg);
extern struct tcp_pcb *__real_tcp_new(void);
extern struct tcp_pcb *__real_tcp_new_ip_type(u8_t type);
extern err_t __real_tcp_bind(struct tcp_pcb *pcb, ip_addr_t *ipaddr, u16_t port);
extern err_t __real_tcp_bind_netif(struct tcp_pcb *pcb, const struct netif *netif);
extern struct tcp_pcb *__real_tcp_listen_with_backlog(struct tcp_pcb *pcb, u8_t backlog);
//extern struct tcp_pcb *__real_tcp_listen_with_backlog_and_err(struct tcp_pcb *pcb, u8_t backlog, err_t *err);
extern void __real_tcp_accept(struct tcp_pcb *pcb, err_t (* accept)(void *arg, struct tcp_pcb *newpcb, err_t err));
extern err_t __real_tcp_connect(struct tcp_pcb *pcb, ip_addr_t *ipaddr, u16_t port, err_t (* connected)(void *arg, struct tcp_pcb *tpcb, err_t err));
extern err_t __real_tcp_write(struct tcp_pcb *pcb, const void *dataptr, u16_t len, u8_t apiflags);
extern void __real_tcp_sent(struct tcp_pcb *pcb, err_t (* sent)(void *arg, struct tcp_pcb *tpcb, u16_t len));
extern void __real_tcp_recv(struct tcp_pcb *pcb, err_t (* recv)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err));
extern void __real_tcp_recved(struct tcp_pcb *pcb, u16_t len);
extern void __real_tcp_poll(struct tcp_pcb *pcb, err_t (* poll)(void *arg, struct tcp_pcb *tpcb), u8_t interval);
extern err_t __real_tcp_close(struct tcp_pcb *pcb);
extern void __real_tcp_abort(struct tcp_pcb *pcb);
extern void __real_tcp_err(struct tcp_pcb *pcb, void (* err)(void *arg, err_t err));
extern err_t __real_tcp_output(struct tcp_pcb *pcb);
extern void __real_tcp_setprio(struct tcp_pcb *pcb, u8_t prio);
extern void __real_tcp_backlog_delayed(struct tcp_pcb* pcb);
extern void __real_tcp_backlog_accepted(struct tcp_pcb* pcb);
extern err_t __real_tcp_shutdown(struct tcp_pcb *pcb, int shut_rx, int shut_tx);
extern struct udp_pcb *__real_udp_new(void);
extern struct udp_pcb *__real_udp_new_ip_type(u8_t type);
extern void __real_udp_remove(struct udp_pcb *pcb);
extern err_t __real_udp_bind(struct udp_pcb *pcb, ip_addr_t *ipaddr, u16_t port);
extern err_t __real_udp_connect(struct udp_pcb *pcb, ip_addr_t *ipaddr, u16_t port);
extern err_t __real_udp_disconnect(struct udp_pcb *pcb);
extern err_t __real_udp_send(struct udp_pcb *pcb, struct pbuf *p);
extern void __real_udp_recv(struct udp_pcb *pcb, void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port), void *recv_arg);
extern err_t __real_udp_sendto(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *dst_ip, u16_t dst_port);
extern err_t __real_udp_sendto_if(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *dst_ip, u16_t dst_port, struct netif *netif);
extern err_t __real_udp_sendto_if_src(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *dst_ip, u16_t dst_port, struct netif *netif, const ip_addr_t *src_ip);
extern void __real_sys_check_timeouts();
extern err_t __real_dns_gethostbyname(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg);
extern err_t __real_dns_gethostbyname_addrtype(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg, u8_t dns_addrtype);
extern struct raw_pcb *__real_raw_new(u8_t proto);
extern struct raw_pcb *__real_raw_new_ip_type(u8_t type, u8_t proto);
extern void __real_raw_recv(struct raw_pcb *pcb, raw_recv_fn recv, void *recv_arg);
extern err_t __real_raw_bind(struct raw_pcb *pcb, const ip_addr_t *ipaddr);
extern err_t __real_raw_sendto(struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *ipaddr);
extern err_t __real_raw_send(struct raw_pcb *pcb, struct pbuf *p);
extern err_t __real_raw_connect(struct raw_pcb *pcb, const ip_addr_t *ipaddr);
extern void __real_raw_remove(struct raw_pcb *pcb);
extern struct netif *__real_netif_add(struct netif *netif, const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip4_addr_t *gw, void *state, netif_init_fn init, netif_input_fn input);
extern void __real_netif_remove(struct netif *netif);
extern err_t __real_ethernet_input(struct pbuf *p, struct netif *netif);
extern int __real_cyw43_wifi_join(cyw43_t *self, size_t ssid_len, const uint8_t *ssid, size_t key_len, const uint8_t *key, uint32_t auth_type, const uint8_t *bssid, uint32_t channel);
extern int __real_cyw43_wifi_leave(cyw43_t *self, int itf);


typedef struct {
    struct pbuf *p;
    s16_t header_size;
    u8_t *ret;
} __pbuf_header_req;

typedef struct {
    struct pbuf *p;
    u8_t *ret;
} __pbuf_free_req;

typedef struct {
    pbuf_layer l;
    u16_t length;
    pbuf_type type;
    struct pbuf **ret;
} __pbuf_alloc_req;

typedef struct {
    struct pbuf *buf;
    const void *dataptr;
    u16_t len;
    err_t *ret;
} __pbuf_take_req;

typedef struct {
    const struct pbuf *p;
    void *dataptr;
    u16_t len;
    u16_t offset;
    u16_t *ret;
} __pbuf_copy_partial_req;

typedef struct {
    struct pbuf *p;
} __pbuf_ref_req;

typedef struct {
    const struct pbuf* p;
    u16_t offset;
    u8_t *ret;
} __pbuf_get_at_req;

typedef struct {
    const struct pbuf *p;
    void *buffer;
    size_t bufsize;
    u16_t len;
    u16_t offset;
    void **ret;
} __pbuf_get_contiguous_req;

typedef struct {
    struct pbuf *head;
    struct pbuf *tail;
} __pbuf_cat_req;

typedef struct {
    struct tcp_pcb *pcb;
    void *arg;
} __tcp_arg_req;

typedef struct {
    struct tcp_pcb **ret;
} __tcp_new_req;

typedef struct {
    u8_t type;
    struct tcp_pcb **ret;
} __tcp_new_ip_type_req;


typedef struct {
    struct tcp_pcb *pcb;
    ip_addr_t *ipaddr;
    u16_t port;
    err_t *ret;
} __tcp_bind_req;

typedef struct {
    struct tcp_pcb *pcb;
    const struct netif *netif;
    err_t *ret;
} __tcp_bind_netif_req;

typedef struct {
    struct tcp_pcb *pcb;
    u8_t backlog;
    struct tcp_pcb **ret;
} __tcp_listen_with_backlog_req;

#if 0
typedef struct {
    struct tcp_pcb *pcb;
    u8_t backlog;
    err_t *err;
    struct tcp_pcb **ret;
} __tcp_listen_with_backlog_and_err_req;
#endif

typedef struct {
    struct tcp_pcb *pcb;
    err_t (* accept)(void *arg, struct tcp_pcb *newpcb, err_t err);
} __tcp_accept_req;

typedef struct {
    struct tcp_pcb *pcb;
    ip_addr_t *ipaddr;
    u16_t port;
    err_t (* connected)(void *arg, struct tcp_pcb *tpcb, err_t err);
    err_t *ret;
} __tcp_connect_req;

typedef struct {
    struct tcp_pcb *pcb;
    const void *dataptr;
    u16_t len;
    u8_t apiflags;
    err_t *ret;
} __tcp_write_req;

typedef struct {
    struct tcp_pcb *pcb;
    err_t (* sent)(void *arg, struct tcp_pcb *tpcb, u16_t len);
} __tcp_sent_req;

typedef struct {
    struct tcp_pcb *pcb;
    err_t (* recv)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
} __tcp_recv_req;

typedef struct {
    struct tcp_pcb *pcb;
    u16_t len;
} __tcp_recved_req;

typedef struct {
    struct tcp_pcb *pcb;
    err_t (* poll)(void *arg, struct tcp_pcb *tpcb);
    u8_t interval;
} __tcp_poll_req;

typedef struct {
    struct tcp_pcb *pcb;
    err_t *ret;
} __tcp_close_req;

typedef struct {
    struct tcp_pcb *pcb;
} __tcp_abort_req;

typedef struct {
    struct tcp_pcb *pcb;
    void (* err)(void *arg, err_t err);
} __tcp_err_req;

typedef struct {
    struct tcp_pcb *pcb;
    err_t *ret;
} __tcp_output_req;

typedef struct {
    struct tcp_pcb *pcb;
    u8_t prio;
} __tcp_setprio_req;

typedef struct {
    struct tcp_pcb* pcb;
} __tcp_backlog_delayed_req;

typedef struct {
    struct tcp_pcb* pcb;
} __tcp_backlog_accepted_req;

typedef struct {
    struct tcp_pcb *pcb;
    int shut_rx;
    int shut_tx;
    err_t *ret;
} __tcp_shutdown_req;

typedef struct {
    struct udp_pcb **ret;
} __udp_new_req;

typedef struct {
    u8_t type;
    struct udp_pcb **ret;
} __udp_new_ip_type_req;

typedef struct {
    struct udp_pcb *pcb;
} __udp_remove_req;

typedef struct {
    struct udp_pcb *pcb;
    ip_addr_t *ipaddr;
    u16_t port;
    err_t *ret;
} __udp_bind_req;

typedef struct {
    struct udp_pcb *pcb;
    ip_addr_t *ipaddr;
    u16_t port;
    err_t *ret;
} __udp_connect_req;

typedef struct {
    struct udp_pcb *pcb;
    err_t *ret;
} __udp_disconnect_req;

typedef struct {
    struct udp_pcb *pcb;
    struct pbuf *p;
    err_t *ret;
} __udp_send_req;

typedef struct {
    struct udp_pcb *pcb;
    void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port);
    void *recv_arg;
} __udp_recv_req;

typedef struct {
    struct udp_pcb *pcb;
    struct pbuf *p;
    const ip_addr_t *dst_ip;
    u16_t dst_port;
    struct netif *netif;
    err_t *ret;
} __udp_sendto_if_req;

typedef struct {
    struct udp_pcb *pcb;
    struct pbuf *p;
    const ip_addr_t *dst_ip;
    u16_t dst_port;
    err_t *ret;
} __udp_sendto_req;

typedef struct {
    struct udp_pcb *pcb;
    struct pbuf *p;
    const ip_addr_t *dst_ip;
    u16_t dst_port;
    struct netif *netif;
    const ip_addr_t *src_ip;
    err_t *ret;
} __udp_sendto_if_src_req;

typedef struct {
    const char *hostname;
    ip_addr_t *addr;
    dns_found_callback found;
    void *callback_arg;
    err_t *ret;
} __dns_gethostbyname_req;

typedef struct {
    const char *hostname;
    ip_addr_t *addr;
    dns_found_callback found;
    void *callback_arg;
    u8_t dns_addrtype;
    err_t *ret;
} __dns_gethostbyname_addrtype_req;

typedef struct {
    u8_t proto;
    struct raw_pcb **ret;
} __raw_new_req;

typedef struct {
    u8_t type;
    u8_t proto;
    struct raw_pcb **ret;
} __raw_new_ip_type_req;

typedef struct {
    struct raw_pcb *pcb;
    const ip_addr_t *ipaddr;
    err_t *ret;
} __raw_connect_req;

typedef struct {
    struct raw_pcb *pcb;
    raw_recv_fn recv;
    void *recv_arg;
} __raw_recv_req;

typedef struct {
    struct raw_pcb *pcb;
    const ip_addr_t *ipaddr;
    err_t *ret;
} __raw_bind_req;

typedef struct {
    struct raw_pcb *pcb;
    struct pbuf *p;
    const ip_addr_t *ipaddr;
    err_t *ret;
} __raw_sendto_req;

typedef struct {
    struct raw_pcb *pcb;
    struct pbuf *p;
    err_t *ret;
} __raw_send_req;

typedef struct {
    struct raw_pcb *pcb;
} __raw_remove_req;

typedef struct {
    struct netif *netif;
    const ip4_addr_t *ipaddr;
    const ip4_addr_t *netmask;
    const ip4_addr_t *gw;
    void *state;
    netif_init_fn init;
    netif_input_fn input;
    struct netif **ret;
} __netif_add_req;

typedef struct {
    struct netif *netif;
} __netif_remove_req;

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
#endif
