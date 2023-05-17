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

#include <pico/mutex.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <lwip/tcp.h>
#include <lwip/dns.h>
#include <lwip/raw.h>
#include <lwip/timeouts.h>
#include <pico/cyw43_arch.h>

class LWIPMutex {
public:
    LWIPMutex() {
        cyw43_arch_lwip_begin();
    }

    ~LWIPMutex() {
        cyw43_arch_lwip_end();
    }
};

extern "C" {

    extern u8_t __real_pbuf_header(struct pbuf *p, s16_t header_size);
    u8_t __wrap_pbuf_header(struct pbuf *p, s16_t header_size) {
        LWIPMutex m;
        return __real_pbuf_header(p, header_size);
    }

    extern u8_t __real_pbuf_free(struct pbuf *p);
    u8_t __wrap_pbuf_free(struct pbuf *p) {
        LWIPMutex m;
        return __real_pbuf_free(p);
    }

    extern struct pbuf *__real_pbuf_alloc(pbuf_layer l, u16_t length, pbuf_type type);
    struct pbuf *__wrap_pbuf_alloc(pbuf_layer l, u16_t length, pbuf_type type) {
        LWIPMutex m;
        return __real_pbuf_alloc(l, length, type);
    }

    extern err_t __real_pbuf_take(struct pbuf *buf, const void *dataptr, u16_t len);
    err_t __wrap_pbuf_take(struct pbuf *buf, const void *dataptr, u16_t len) {
        LWIPMutex m;
        return __real_pbuf_take(buf, dataptr, len);
    }

    extern u16_t __real_pbuf_copy_partial(const struct pbuf *p, void *dataptr, u16_t len, u16_t offset);
    u16_t __wrap_pbuf_copy_partial(const struct pbuf *p, void *dataptr, u16_t len, u16_t offset) {
        LWIPMutex m;
        return __real_pbuf_copy_partial(p, dataptr, len, offset);
    }

    extern void __real_pbuf_ref(struct pbuf *p);
    void __wrap_pbuf_ref(struct pbuf *p) {
        LWIPMutex m;
        __real_pbuf_ref(p);
    }

    extern u8_t __real_pbuf_get_at(const struct pbuf* p, u16_t offset);
    u8_t __wrap_pbuf_get_at(const struct pbuf* p, u16_t offset) {
        LWIPMutex m;
        return __real_pbuf_get_at(p, offset);
    }

    extern void *__real_pbuf_get_contiguous(const struct pbuf *p, void *buffer, size_t bufsize, u16_t len, u16_t offset);
    void *__wrap_pbuf_get_contiguous(const struct pbuf *p, void *buffer, size_t bufsize, u16_t len, u16_t offset) {
        LWIPMutex m;
        return __real_pbuf_get_contiguous(p, buffer, bufsize, len, offset);
    }

    extern void __real_pbuf_cat(struct pbuf *head, struct pbuf *tail);
    void __wrap_pbuf_cat(struct pbuf *head, struct pbuf *tail) {
        LWIPMutex m;
        __real_pbuf_cat(head, tail);
    }

    extern void __real_tcp_arg(struct tcp_pcb *pcb, void *arg);
    void __wrap_tcp_arg(struct tcp_pcb *pcb, void *arg) {
        LWIPMutex m;
        __real_tcp_arg(pcb, arg);
    }

    extern struct tcp_pcb *__real_tcp_new(void);
    struct tcp_pcb *__wrap_tcp_new(void) {
        LWIPMutex m;
        return __real_tcp_new();
    }

    extern err_t __real_tcp_bind(struct tcp_pcb *pcb, ip_addr_t *ipaddr, u16_t port);
    err_t __wrap_tcp_bind(struct tcp_pcb *pcb, ip_addr_t *ipaddr, u16_t port) {
        LWIPMutex m;
        return __real_tcp_bind(pcb, ipaddr, port);
    }

    extern struct tcp_pcb *__real_tcp_listen(struct tcp_pcb *pcb);
    struct tcp_pcb *__wrap_tcp_listen(struct tcp_pcb *pcb) {
        LWIPMutex m;
        return __real_tcp_listen(pcb);
    }

    extern struct tcp_pcb *__real_tcp_listen_with_backlog(struct tcp_pcb *pcb, u8_t backlog);
    struct tcp_pcb *__wrap_tcp_listen_with_backlog(struct tcp_pcb *pcb, u8_t backlog) {
        LWIPMutex m;
        return __real_tcp_listen_with_backlog(pcb, backlog);
    }

    extern void __real_tcp_accept(struct tcp_pcb *pcb, err_t (* accept)(void *arg, struct tcp_pcb *newpcb, err_t err));
    void __wrap_tcp_accept(struct tcp_pcb *pcb, err_t (* accept)(void *arg, struct tcp_pcb *newpcb, err_t err)) {
        LWIPMutex m;
        __real_tcp_accept(pcb, accept);
    }

    extern err_t __real_tcp_connect(struct tcp_pcb *pcb, ip_addr_t *ipaddr, u16_t port, err_t (* connected)(void *arg, struct tcp_pcb *tpcb, err_t err));
    err_t __wrap_tcp_connect(struct tcp_pcb *pcb, ip_addr_t *ipaddr, u16_t port, err_t (* connected)(void *arg, struct tcp_pcb *tpcb, err_t err)) {
        LWIPMutex m;
        return __real_tcp_connect(pcb, ipaddr, port, connected);
    }

    extern err_t __real_tcp_write(struct tcp_pcb *pcb, const void *dataptr, u16_t len, u8_t apiflags);
    err_t __wrap_tcp_write(struct tcp_pcb *pcb, const void *dataptr, u16_t len, u8_t apiflags) {
        LWIPMutex m;
        return __real_tcp_write(pcb, dataptr, len, apiflags);
    }

    extern void __real_tcp_sent(struct tcp_pcb *pcb, err_t (* sent)(void *arg, struct tcp_pcb *tpcb, u16_t len));
    void __wrap_tcp_sent(struct tcp_pcb *pcb, err_t (* sent)(void *arg, struct tcp_pcb *tpcb, u16_t len)) {
        LWIPMutex m;
        __real_tcp_sent(pcb, sent);
    }

    extern void __real_tcp_recv(struct tcp_pcb *pcb, err_t (* recv)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err));
    void __wrap_tcp_recv(struct tcp_pcb *pcb, err_t (* recv)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)) {
        LWIPMutex m;
        __real_tcp_recv(pcb, recv);
    }

    extern void __real_tcp_recved(struct tcp_pcb *pcb, u16_t len);
    void __wrap_tcp_recved(struct tcp_pcb *pcb, u16_t len) {
        LWIPMutex m;
        __real_tcp_recved(pcb, len);
    }

    extern void __real_tcp_poll(struct tcp_pcb *pcb, err_t (* poll)(void *arg, struct tcp_pcb *tpcb), u8_t interval);
    void __wrap_tcp_poll(struct tcp_pcb *pcb, err_t (* poll)(void *arg, struct tcp_pcb *tpcb), u8_t interval) {
        LWIPMutex m;
        __real_tcp_poll(pcb, poll, interval);
    }

    extern err_t __real_tcp_close(struct tcp_pcb *pcb);
    err_t __wrap_tcp_close(struct tcp_pcb *pcb) {
        LWIPMutex m;
        return __real_tcp_close(pcb);
    }

    extern void __real_tcp_abort(struct tcp_pcb *pcb);
    void __wrap_tcp_abort(struct tcp_pcb *pcb) {
        LWIPMutex m;
        __real_tcp_abort(pcb);
    }

    extern void __real_tcp_err(struct tcp_pcb *pcb, void (* err)(void *arg, err_t err));
    void __wrap_tcp_err(struct tcp_pcb *pcb, void (* err)(void *arg, err_t err)) {
        LWIPMutex m;
        __real_tcp_err(pcb, err);
    }

    extern err_t __real_tcp_output(struct tcp_pcb *pcb);
    err_t __wrap_tcp_output(struct tcp_pcb *pcb) {
        LWIPMutex m;
        return __real_tcp_output(pcb);
    }

    extern void __real_tcp_setprio(struct tcp_pcb *pcb, u8_t prio);
    void __wrap_tcp_setprio(struct tcp_pcb *pcb, u8_t prio) {
        LWIPMutex m;
        return __real_tcp_setprio(pcb, prio);
    }

    extern void __real_tcp_backlog_delayed(struct tcp_pcb* pcb);
    void __wrap_tcp_backlog_delayed(struct tcp_pcb* pcb) {
        LWIPMutex m;
        return __real_tcp_backlog_delayed(pcb);
    }

    extern void __real_tcp_backlog_accepted(struct tcp_pcb* pcb);
    void __wrap_tcp_backlog_accepted(struct tcp_pcb* pcb) {
        LWIPMutex m;
        return __real_tcp_backlog_accepted(pcb);
    }
    extern struct udp_pcb *__real_udp_new(void);
    struct udp_pcb *__wrap_udp_new(void) {
        LWIPMutex m;
        return __real_udp_new();
    }

    extern void __real_udp_remove(struct udp_pcb *pcb);
    void __wrap_udp_remove(struct udp_pcb *pcb) {
        LWIPMutex m;
        __real_udp_remove(pcb);
    }

    extern err_t __real_udp_bind(struct udp_pcb *pcb, ip_addr_t *ipaddr, u16_t port);
    err_t __wrap_udp_bind(struct udp_pcb *pcb, ip_addr_t *ipaddr, u16_t port) {
        LWIPMutex m;
        return __real_udp_bind(pcb, ipaddr, port);
    }

    extern err_t __real_udp_connect(struct udp_pcb *pcb, ip_addr_t *ipaddr, u16_t port);
    err_t __wrap_udp_connect(struct udp_pcb *pcb, ip_addr_t *ipaddr, u16_t port) {
        LWIPMutex m;
        return __real_udp_connect(pcb, ipaddr, port);
    }

    extern err_t __real_udp_disconnect(struct udp_pcb *pcb);
    err_t __wrap_udp_disconnect(struct udp_pcb *pcb) {
        LWIPMutex m;
        return __real_udp_disconnect(pcb);
    }

    extern err_t __real_udp_send(struct udp_pcb *pcb, struct pbuf *p);
    err_t __wrap_udp_send(struct udp_pcb *pcb, struct pbuf *p) {
        LWIPMutex m;
        return __real_udp_send(pcb, p);
    }

    extern void __real_udp_recv(struct udp_pcb *pcb, void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port), void *recv_arg);
    void __wrap_udp_recv(struct udp_pcb *pcb, void (* recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port), void *recv_arg) {
        LWIPMutex m;
        __real_udp_recv(pcb, recv, recv_arg);
    }

    extern err_t __real_udp_sendto_if(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *dst_ip, u16_t dst_port, struct netif *netif);
    err_t __wrap_udp_sendto_if(struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *dst_ip, u16_t dst_port, struct netif *netif) {
        LWIPMutex m;
        return __real_udp_sendto_if(pcb, p, dst_ip, dst_port, netif);
    }

    // sys_check_timeouts is special case because the async process will call it.  If we're already in a timeout check, just do a noop
    extern void __real_sys_check_timeouts();
    void __wrap_sys_check_timeouts(void) {
        LWIPMutex m;
        __real_sys_check_timeouts();
    }

    extern err_t __real_dns_gethostbyname(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg);
    err_t __wrap_dns_gethostbyname(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg) {
        LWIPMutex m;
        return __real_dns_gethostbyname(hostname, addr, found, callback_arg);
    }

    extern err_t __real_dns_gethostbyname_addrtype(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg, u8_t dns_addrtype);
    err_t __wrap_dns_gethostbyname_addrtype(const char *hostname, ip_addr_t *addr, dns_found_callback found, void *callback_arg, u8_t dns_addrtype) {
        LWIPMutex m;
        return __real_dns_gethostbyname_addrtype(hostname, addr, found, callback_arg, dns_addrtype);
    }

    extern struct raw_pcb *__real_raw_new(u8_t proto);
    struct raw_pcb *__wrap_raw_new(u8_t proto) {
        LWIPMutex m;
        return __real_raw_new(proto);
    }

    extern void __real_raw_recv(struct raw_pcb *pcb, raw_recv_fn recv, void *recv_arg);
    void __wrap_raw_recv(struct raw_pcb *pcb, raw_recv_fn recv, void *recv_arg) {
        LWIPMutex m;
        __real_raw_recv(pcb, recv, recv_arg);
    }

    extern err_t __real_raw_bind(struct raw_pcb *pcb, const ip_addr_t *ipaddr);
    err_t __wrap_raw_bind(struct raw_pcb *pcb, const ip_addr_t *ipaddr) {
        LWIPMutex m;
        return __real_raw_bind(pcb, ipaddr);
    }

    extern err_t __real_raw_sendto(struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *ipaddr);
    err_t __wrap_raw_sendto(struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *ipaddr) {
        LWIPMutex m;
        return __real_raw_sendto(pcb, p, ipaddr);
    }

    extern void __real_raw_remove(struct raw_pcb *pcb);
    void __wrap_raw_remove(struct raw_pcb *pcb) {
        LWIPMutex m;
        __real_raw_remove(pcb);
    }

}; // extern "C"
