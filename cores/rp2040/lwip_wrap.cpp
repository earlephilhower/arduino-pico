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

#if defined(__FREERTOS)
#define __isFreeRTOS 1
#else
#define __isFreeRTOS 0
#endif

//auto_init_recursive_mutex(__lwipMutex); // Only for case with no Ethernet or PicoW, but still doing LWIP (PPP?)
recursive_mutex_t __lwipMutex;

extern "C" {

    extern void __lwip(__lwip_op op, void *req) __attribute((weak));
    extern bool __isLWIPThread();

    static XoshiroCpp::Xoshiro256PlusPlus *_lwip_rng = nullptr;
    // Random number generator for LWIP
    unsigned long __lwip_rand() {
        return (unsigned long)(*_lwip_rng)();
    }

    // Avoid calling lwip_init multiple times
    extern void __real_lwip_init();
    void __wrap_lwip_init() {
        if (!_lwip_rng) {
            recursive_mutex_init(&__lwipMutex);
            _lwip_rng = new XoshiroCpp::Xoshiro256PlusPlus(micros() * rp2040.getCycleCount());
            __real_lwip_init();
        }
    }

    extern u8_t __real_pbuf_header(struct pbuf *p, s16_t header_size);
    u8_t __wrap_pbuf_header(struct pbuf *p, s16_t header_size) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            u8_t ret;
            __pbuf_header_req req = { p, header_size, &ret };
            __lwip(__pbuf_header, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_pbuf_header(p, header_size);
    }

    extern u8_t __real_pbuf_free(struct pbuf *p);
    u8_t __wrap_pbuf_free(struct pbuf *p) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            u8_t ret;
            __pbuf_free_req req = { p, &ret };
            __lwip(__pbuf_free, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_pbuf_free(p);
    }

    extern struct pbuf *__real_pbuf_alloc(pbuf_layer l, u16_t length, pbuf_type type);
    struct pbuf *__wrap_pbuf_alloc(pbuf_layer l, u16_t length, pbuf_type type) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            struct pbuf *ret;
            __pbuf_alloc_req req = {l, length, type, &ret };
            __lwip(__pbuf_alloc, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_pbuf_alloc(l, length, type);
    }

    extern err_t __real_pbuf_take(struct pbuf *buf, const void *dataptr, u16_t len);
    err_t __wrap_pbuf_take(struct pbuf *buf, const void *dataptr, u16_t len) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __pbuf_take_req req = { buf, dataptr, len, &ret };
            __lwip(__pbuf_take, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_pbuf_take(buf, dataptr, len);
    }

    extern u16_t __real_pbuf_copy_partial(const struct pbuf *p, void *dataptr, u16_t len, u16_t offset);
    u16_t __wrap_pbuf_copy_partial(const struct pbuf *p, void *dataptr, u16_t len, u16_t offset) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            u16_t ret;
            __pbuf_copy_partial_req req = { p, dataptr, len, offset, &ret };
            __lwip(__pbuf_copy_partial, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_pbuf_copy_partial(p, dataptr, len, offset);
    }

    extern void __real_pbuf_ref(struct pbuf *p);
    void __wrap_pbuf_ref(struct pbuf *p) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __pbuf_ref_req req = { p };
            __lwip(__pbuf_ref, &req);
            return;
        }
        LWIPMutex m;
        __real_pbuf_ref(p);
    }

    extern u8_t __real_pbuf_get_at(const struct pbuf* p, u16_t offset);
    u8_t __wrap_pbuf_get_at(const struct pbuf* p, u16_t offset) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            u8_t ret;
            __pbuf_get_at_req req = { p, offset, &ret };
            __lwip(__pbuf_get_at, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_pbuf_get_at(p, offset);
    }

    extern void *__real_pbuf_get_contiguous(const struct pbuf *p, void *buffer, size_t bufsize, u16_t len, u16_t offset);
    void *__wrap_pbuf_get_contiguous(const struct pbuf *p, void *buffer, size_t bufsize, u16_t len, u16_t offset) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            void *ret;
            __pbuf_get_contiguous_req req = { p, buffer, bufsize, len, offset, &ret };
            __lwip(__pbuf_get_contiguous, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_pbuf_get_contiguous(p, buffer, bufsize, len, offset);
    }

    extern void __real_pbuf_cat(struct pbuf *head, struct pbuf *tail);
    void __wrap_pbuf_cat(struct pbuf *head, struct pbuf *tail) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __pbuf_cat_req req = { head, tail };
            __lwip(__pbuf_cat, &req);
            return;
        }
        LWIPMutex m;
        __real_pbuf_cat(head, tail);
    }

    extern void __real_tcp_arg(struct tcp_pcb *pcb, void *arg);
    void __wrap_tcp_arg(struct tcp_pcb *pcb, void *arg) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __tcp_arg_req req = { pcb, arg };
            __lwip(__tcp_arg, &req);
            return;
        }
        LWIPMutex m;
        __real_tcp_arg(pcb, arg);
    }

    extern struct tcp_pcb *__real_tcp_new(void);
    struct tcp_pcb *__wrap_tcp_new(void) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            struct tcp_pcb *ret;
            __tcp_new_req req = { &ret };
            __lwip(__tcp_new, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_tcp_new();
    }

    extern err_t __real_tcp_bind(struct tcp_pcb *pcb, ip_addr_t *ipaddr, u16_t port);
    err_t __wrap_tcp_bind(struct tcp_pcb *pcb, ip_addr_t *ipaddr, u16_t port) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __tcp_bind_req req = { pcb, ipaddr, port, &ret };
            __lwip(__tcp_bind, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_tcp_bind(pcb, ipaddr, port);
    }

    extern struct tcp_pcb *__real_tcp_listen_with_backlog(struct tcp_pcb *pcb, u8_t backlog);
    struct tcp_pcb *__wrap_tcp_listen_with_backlog(struct tcp_pcb *pcb, u8_t backlog) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            struct tcp_pcb *ret;
            __tcp_listen_with_backlog_req req = { pcb, backlog, &ret };
            __lwip(__tcp_listen_with_backlog, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_tcp_listen_with_backlog(pcb, backlog);
    }

    extern void __real_tcp_accept(struct tcp_pcb *pcb, err_t (* accept)(void *arg, struct tcp_pcb *newpcb, err_t err));
    void __wrap_tcp_accept(struct tcp_pcb *pcb, err_t (* accept)(void *arg, struct tcp_pcb *newpcb, err_t err)) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __tcp_accept_req req = { pcb, accept };
            __lwip(__tcp_accept, &req);
            return;
        }
        LWIPMutex m;
        __real_tcp_accept(pcb, accept);
    }

    extern err_t __real_tcp_connect(struct tcp_pcb *pcb, ip_addr_t *ipaddr, u16_t port, err_t (* connected)(void *arg, struct tcp_pcb *tpcb, err_t err));
    err_t __wrap_tcp_connect(struct tcp_pcb *pcb, ip_addr_t *ipaddr, u16_t port, err_t (* connected)(void *arg, struct tcp_pcb *tpcb, err_t err)) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __tcp_connect_req req = { pcb, ipaddr, port, connected, &ret };
            __lwip(__tcp_connect, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_tcp_connect(pcb, ipaddr, port, connected);
    }

    extern err_t __real_tcp_write(struct tcp_pcb *pcb, const void *dataptr, u16_t len, u8_t apiflags);
    err_t __wrap_tcp_write(struct tcp_pcb *pcb, const void *dataptr, u16_t len, u8_t apiflags) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __tcp_write_req req = { pcb, dataptr, len, apiflags, &ret };
            __lwip(__tcp_write, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_tcp_write(pcb, dataptr, len, apiflags);
    }

    extern void __real_tcp_sent(struct tcp_pcb *pcb, err_t (* sent)(void *arg, struct tcp_pcb *tpcb, u16_t len));
    void __wrap_tcp_sent(struct tcp_pcb *pcb, err_t (* sent)(void *arg, struct tcp_pcb *tpcb, u16_t len)) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __tcp_sent_req req = { pcb, sent };
            __lwip(__tcp_sent, &req);
            return;
        }
        LWIPMutex m;
        __real_tcp_sent(pcb, sent);
    }

    extern void __real_tcp_recv(struct tcp_pcb *pcb, err_t (* recv)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err));
    void __wrap_tcp_recv(struct tcp_pcb *pcb, err_t (* recv)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __tcp_recv_req req = { pcb, recv };
            __lwip(__tcp_recv, &req);
            return;
        }
        LWIPMutex m;
        __real_tcp_recv(pcb, recv);
    }

    extern void __real_tcp_recved(struct tcp_pcb *pcb, u16_t len);
    void __wrap_tcp_recved(struct tcp_pcb *pcb, u16_t len) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __tcp_recved_req req = { pcb, len };
            __lwip(__tcp_recved, &req);
            return;
        }
        LWIPMutex m;
        __real_tcp_recved(pcb, len);
    }

    extern void __real_tcp_poll(struct tcp_pcb *pcb, err_t (* poll)(void *arg, struct tcp_pcb *tpcb), u8_t interval);
    void __wrap_tcp_poll(struct tcp_pcb *pcb, err_t (* poll)(void *arg, struct tcp_pcb *tpcb), u8_t interval) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __tcp_poll_req req = { pcb, poll, interval };
            __lwip(__tcp_poll, &req);
            return;
        }
        LWIPMutex m;
        __real_tcp_poll(pcb, poll, interval);
    }

    extern err_t __real_tcp_close(struct tcp_pcb *pcb);
    err_t __wrap_tcp_close(struct tcp_pcb *pcb) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __tcp_close_req req = { pcb, &ret };
            __lwip(__tcp_close, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_tcp_close(pcb);
    }

    extern void __real_tcp_abort(struct tcp_pcb *pcb);
    void __wrap_tcp_abort(struct tcp_pcb *pcb) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __tcp_abort_req req = { pcb };
            __lwip(__tcp_abort, &req);
            return;
        }
        LWIPMutex m;
        __real_tcp_abort(pcb);
    }

    extern void __real_tcp_err(struct tcp_pcb *pcb, void (* err)(void *arg, err_t err));
    void __wrap_tcp_err(struct tcp_pcb *pcb, void (* err)(void *arg, err_t err)) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __tcp_err_req req = { pcb, err };
            __lwip(__tcp_err, &req);
            return;
        }
        LWIPMutex m;
        __real_tcp_err(pcb, err);
    }

    extern err_t __real_tcp_output(struct tcp_pcb *pcb);
    err_t __wrap_tcp_output(struct tcp_pcb *pcb) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __tcp_output_req req = { pcb, &ret };
            __lwip(__tcp_output, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_tcp_output(pcb);
    }

    extern void __real_tcp_setprio(struct tcp_pcb *pcb, u8_t prio);
    void __wrap_tcp_setprio(struct tcp_pcb *pcb, u8_t prio) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __tcp_setprio_req req = { pcb, prio };
            __lwip(__tcp_setprio, &req);
            return;
        }
        LWIPMutex m;
        __real_tcp_setprio(pcb, prio);
    }

    extern void __real_tcp_backlog_delayed(struct tcp_pcb* pcb);
    void __wrap_tcp_backlog_delayed(struct tcp_pcb* pcb) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __tcp_backlog_delayed_req req = { pcb };
            __lwip(__tcp_backlog_delayed, &req);
            return;
        }
        LWIPMutex m;
        __real_tcp_backlog_delayed(pcb);
    }

    extern void __real_tcp_backlog_accepted(struct tcp_pcb* pcb);
    void __wrap_tcp_backlog_accepted(struct tcp_pcb* pcb) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __tcp_backlog_accepted_req req = { pcb };
            __lwip(__tcp_backlog_accepted, &req);
            return;
        }
        LWIPMutex m;
        __real_tcp_backlog_accepted(pcb);
    }
    extern struct udp_pcb *__real_udp_new(void);
    struct udp_pcb *__wrap_udp_new(void) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            struct udp_pcb *ret;
            __udp_new_req req = { &ret };
            __lwip(__udp_new, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_udp_new();
    }
    extern struct udp_pcb *__real_udp_new_ip_type(u8_t type);
    struct udp_pcb *__wrap_udp_new_ip_type(u8_t type) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            struct udp_pcb *ret;
            __udp_new_ip_type_req req = { type, &ret };
            __lwip(__udp_new_ip_type, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_udp_new_ip_type(type);
    }


    extern void __real_udp_remove(struct udp_pcb *pcb);
    void __wrap_udp_remove(struct udp_pcb *pcb) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __udp_remove_req req = { pcb };
            __lwip(__udp_remove, &req);
            return;
        }
        LWIPMutex m;
        __real_udp_remove(pcb);
    }

    extern err_t __real_udp_bind(struct udp_pcb *pcb, ip_addr_t *ipaddr, u16_t port);
    err_t __wrap_udp_bind(struct udp_pcb *pcb, ip_addr_t *ipaddr, u16_t port) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __udp_bind_req req = { pcb, ipaddr, port, &ret };
            __lwip(__udp_bind, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_udp_bind(pcb, ipaddr, port);
    }

    extern err_t __real_udp_connect(struct udp_pcb *pcb, ip_addr_t *ipaddr, u16_t port);
    err_t __wrap_udp_connect(struct udp_pcb *pcb, ip_addr_t *ipaddr, u16_t port) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __udp_connect_req req = { pcb, ipaddr, port, &ret };
            __lwip(__udp_connect, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_udp_connect(pcb, ipaddr, port);
    }

    extern err_t __real_udp_disconnect(struct udp_pcb *pcb);
    err_t __wrap_udp_disconnect(struct udp_pcb *pcb) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __udp_disconnect_req req = { pcb, &ret };
            __lwip(__udp_disconnect, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_udp_disconnect(pcb);
    }

    extern err_t __real_udp_send(struct udp_pcb *pcb, struct pbuf *p);
    err_t __wrap_udp_send(struct udp_pcb *pcb, struct pbuf *p) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __udp_send_req req = { pcb, p, &ret };
            __lwip(__udp_send, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_udp_send(pcb, p);
    }

    extern void __real_udp_recv(struct udp_pcb *pcb, void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port), void *recv_arg);
    void __wrap_udp_recv(struct udp_pcb *pcb, void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port), void *recv_arg) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __udp_recv_req req = { pcb, recv, recv_arg };
            __lwip(__udp_recv, &req);
            return;
        }
        LWIPMutex m;
        __real_udp_recv(pcb, recv, recv_arg);
    }

    extern err_t __real_udp_sendto(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *dst_ip, u16_t dst_port);
    err_t __wrap_udp_sendto(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *dst_ip, u16_t dst_port) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __udp_sendto_req req = { pcb, p, dst_ip, dst_port, &ret };
            __lwip(__udp_sendto, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_udp_sendto(pcb, p, dst_ip, dst_port);
    }

    extern err_t __real_udp_sendto_if(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *dst_ip, u16_t dst_port, struct netif *netif);
    err_t __wrap_udp_sendto_if(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *dst_ip, u16_t dst_port, struct netif *netif) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __udp_sendto_if_req req = { pcb, p, dst_ip, dst_port, netif, &ret };
            __lwip(__udp_sendto_if, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_udp_sendto_if(pcb, p, dst_ip, dst_port, netif);
    }

    extern err_t __real_udp_sendto_if_src(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *dst_ip, u16_t dst_port, struct netif *netif, const ip_addr_t *src_ip);
    err_t __wrap_udp_sendto_if_src(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *dst_ip, u16_t dst_port, struct netif *netif, const ip_addr_t *src_ip) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __udp_sendto_if_src_req req = { pcb, p, dst_ip, dst_port, netif, src_ip, &ret };
            __lwip(__udp_sendto_if_src, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_udp_sendto_if_src(pcb, p, dst_ip, dst_port, netif, src_ip);
    }

    // sys_check_timeouts is special case because the async process will call it.  If we're already in a timeout check, just do a noop
    extern void __real_sys_check_timeouts();
    void __wrap_sys_check_timeouts(void) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __lwip(__sys_check_timeouts, nullptr);
            return;
        }
        LWIPMutex m;
        __real_sys_check_timeouts();
    }

    extern err_t __real_dns_gethostbyname(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg);
    err_t __wrap_dns_gethostbyname(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __dns_gethostbyname_req req = { hostname, addr, found, callback_arg, &ret };
            __lwip(__dns_gethostbyname, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_dns_gethostbyname(hostname, addr, found, callback_arg);
    }

    extern err_t __real_dns_gethostbyname_addrtype(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg, u8_t dns_addrtype);
    err_t __wrap_dns_gethostbyname_addrtype(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg, u8_t dns_addrtype) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __dns_gethostbyname_addrtype_req req = { hostname, addr, found, callback_arg, dns_addrtype, &ret };
            __lwip(__dns_gethostbyname_addrtype, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_dns_gethostbyname_addrtype(hostname, addr, found, callback_arg, dns_addrtype);
    }

    extern struct raw_pcb *__real_raw_new(u8_t proto);
    struct raw_pcb *__wrap_raw_new(u8_t proto) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            struct raw_pcb *ret;
            __raw_new_req req = { proto, &ret };
            __lwip(__raw_new, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_raw_new(proto);
    }

    extern void __real_raw_recv(struct raw_pcb *pcb, raw_recv_fn recv, void *recv_arg);
    void __wrap_raw_recv(struct raw_pcb *pcb, raw_recv_fn recv, void *recv_arg) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __raw_recv_req req = { pcb, recv, recv_arg };
            __lwip(__raw_recv, &req);
            return;
        }
        LWIPMutex m;
        __real_raw_recv(pcb, recv, recv_arg);
    }

    extern err_t __real_raw_bind(struct raw_pcb *pcb, const ip_addr_t *ipaddr);
    err_t __wrap_raw_bind(struct raw_pcb *pcb, const ip_addr_t *ipaddr) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __raw_bind_req req = { pcb, ipaddr, &ret };
            __lwip(__raw_bind, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_raw_bind(pcb, ipaddr);
    }

    extern err_t __real_raw_sendto(struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *ipaddr);
    err_t __wrap_raw_sendto(struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *ipaddr) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            err_t ret;
            __raw_sendto_req req = { pcb, p, ipaddr, &ret };
            __lwip(__raw_sendto, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_raw_sendto(pcb, p, ipaddr);
    }

    extern void __real_raw_remove(struct raw_pcb *pcb);
    void __wrap_raw_remove(struct raw_pcb *pcb) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __raw_remove_req req = { pcb };
            __lwip(__raw_remove, &req);
            return;
        }
        LWIPMutex m;
        __real_raw_remove(pcb);
    }

    extern struct netif *__real_netif_add(struct netif *netif, const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip4_addr_t *gw, void *state, netif_init_fn init, netif_input_fn input);
    struct netif *__wrap_netif_add(struct netif *netif, const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip4_addr_t *gw, void *state, netif_init_fn init, netif_input_fn input) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            struct netif *ret;
            __netif_add_req req = { netif, ipaddr, netmask, gw, state, init, input, &ret };
            __lwip(__netif_add, &req);
            return ret;
        }
        LWIPMutex m;
        return __real_netif_add(netif, ipaddr, netmask, gw, state, init, input);
    }

    extern void __real_netif_remove(struct netif *netif);
    void __wrap_netif_remove(struct netif *netif) {
        if (__isFreeRTOS && !__isLWIPThread()) {
            __netif_remove_req req = { netif };
            __lwip(__netif_remove, &req);
            return;
        }
        LWIPMutex m;
        __real_netif_remove(netif);
    }

}; // extern "C"
