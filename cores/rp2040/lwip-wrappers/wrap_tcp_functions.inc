// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_EVENT_API
err_t __wrap_lwip_tcp_event(void * arg,struct tcp_pcb * pcb,enum lwip_event,struct pbuf * p,u16_t size,err_t err) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __lwip_tcp_event_req req = { arg, pcb, lwip_event, p, size, err, &ret };
        __lwip(__lwip_tcp_event, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_lwip_tcp_event(arg, pcb, lwip_event, p, size, err);
}

#endif
void __wrap_tcp_abort(struct tcp_pcb * pcb) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_abort_req req = { pcb };
        __lwip(__tcp_abort, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_abort(pcb);
}

void __wrap_tcp_accept(struct tcp_pcb * pcb,tcp_accept_fn accept) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_accept_req req = { pcb, accept };
        __lwip(__tcp_accept, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_accept(pcb, accept);
}

void __wrap_tcp_arg(struct tcp_pcb * pcb,void * arg) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_arg_req req = { pcb, arg };
        __lwip(__tcp_arg, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_arg(pcb, arg);
}

void __wrap_tcp_backlog_accepted(struct tcp_pcb * pcb) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_backlog_accepted_req req = { pcb };
        __lwip(__tcp_backlog_accepted, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_backlog_accepted(pcb);
}

void __wrap_tcp_backlog_delayed(struct tcp_pcb * pcb) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_backlog_delayed_req req = { pcb };
        __lwip(__tcp_backlog_delayed, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_backlog_delayed(pcb);
}

err_t __wrap_tcp_bind(struct tcp_pcb * pcb,const ip_addr_t * ipaddr,u16_t port) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __tcp_bind_req req = { pcb, ipaddr, port, &ret };
        __lwip(__tcp_bind, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_bind(pcb, ipaddr, port);
}

void __wrap_tcp_bind_netif(struct tcp_pcb * pcb,const struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_bind_netif_req req = { pcb, netif };
        __lwip(__tcp_bind_netif, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_bind_netif(pcb, netif);
}

err_t __wrap_tcp_close(struct tcp_pcb * pcb) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __tcp_close_req req = { pcb, &ret };
        __lwip(__tcp_close, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_close(pcb);
}

err_t __wrap_tcp_connect(struct tcp_pcb * pcb,const ip_addr_t * ipaddr,u16_t port,tcp_connected_fn connected) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __tcp_connect_req req = { pcb, ipaddr, port, connected, &ret };
        __lwip(__tcp_connect, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_connect(pcb, ipaddr, port, connected);
}

void __wrap_tcp_err(struct tcp_pcb * pcb,tcp_err_fn err) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_err_req req = { pcb, err };
        __lwip(__tcp_err, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_err(pcb, err);
}

#if LWIP_TCP_PCB_NUM_EXT_ARGS
u8_t __wrap_tcp_ext_arg_alloc_id() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __tcp_ext_arg_alloc_id_req req = { &ret };
        __lwip(__tcp_ext_arg_alloc_id, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_ext_arg_alloc_id();
}

#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
void * __wrap_tcp_ext_arg_get(const struct tcp_pcb * pcb,u8_t id) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        void * ret;
        __tcp_ext_arg_get_req req = { pcb, id, &ret };
        __lwip(__tcp_ext_arg_get, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_ext_arg_get(pcb, id);
}

#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
void __wrap_tcp_ext_arg_set(struct tcp_pcb * pcb,u8_t id,void * arg) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_ext_arg_set_req req = { pcb, id, arg };
        __lwip(__tcp_ext_arg_set, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_ext_arg_set(pcb, id, arg);
}

#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
void __wrap_tcp_ext_arg_set_callbacks(struct tcp_pcb * pcb,u8_t id,const struct tcp_ext_arg_callbacks * const callbacks) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_ext_arg_set_callbacks_req req = { pcb, id, callbacks };
        __lwip(__tcp_ext_arg_set_callbacks, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_ext_arg_set_callbacks(pcb, id, callbacks);
}

#endif
struct tcp_pcb * __wrap_tcp_listen_with_backlog(struct tcp_pcb * pcb,u8_t backlog) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct tcp_pcb * ret;
        __tcp_listen_with_backlog_req req = { pcb, backlog, &ret };
        __lwip(__tcp_listen_with_backlog, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_listen_with_backlog(pcb, backlog);
}

struct tcp_pcb * __wrap_tcp_listen_with_backlog_and_err(struct tcp_pcb * pcb,u8_t backlog,err_t * err) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct tcp_pcb * ret;
        __tcp_listen_with_backlog_and_err_req req = { pcb, backlog, err, &ret };
        __lwip(__tcp_listen_with_backlog_and_err, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_listen_with_backlog_and_err(pcb, backlog, err);
}

struct tcp_pcb * __wrap_tcp_new() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct tcp_pcb * ret;
        __tcp_new_req req = { &ret };
        __lwip(__tcp_new, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_new();
}

struct tcp_pcb * __wrap_tcp_new_ip_type(u8_t type) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct tcp_pcb * ret;
        __tcp_new_ip_type_req req = { type, &ret };
        __lwip(__tcp_new_ip_type, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_new_ip_type(type);
}

err_t __wrap_tcp_output(struct tcp_pcb * pcb) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __tcp_output_req req = { pcb, &ret };
        __lwip(__tcp_output, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_output(pcb);
}

void __wrap_tcp_poll(struct tcp_pcb * pcb,tcp_poll_fn poll,u8_t interval) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_poll_req req = { pcb, poll, interval };
        __lwip(__tcp_poll, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_poll(pcb, poll, interval);
}

void __wrap_tcp_recv(struct tcp_pcb * pcb,tcp_recv_fn recv) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_recv_req req = { pcb, recv };
        __lwip(__tcp_recv, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_recv(pcb, recv);
}

void __wrap_tcp_recved(struct tcp_pcb * pcb,u16_t len) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_recved_req req = { pcb, len };
        __lwip(__tcp_recved, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_recved(pcb, len);
}

void __wrap_tcp_sent(struct tcp_pcb * pcb,tcp_sent_fn sent) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_sent_req req = { pcb, sent };
        __lwip(__tcp_sent, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_sent(pcb, sent);
}

void __wrap_tcp_setprio(struct tcp_pcb * pcb,u8_t prio) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __tcp_setprio_req req = { pcb, prio };
        __lwip(__tcp_setprio, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_tcp_setprio(pcb, prio);
}

err_t __wrap_tcp_shutdown(struct tcp_pcb * pcb,int shut_rx,int shut_tx) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __tcp_shutdown_req req = { pcb, shut_rx, shut_tx, &ret };
        __lwip(__tcp_shutdown, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_shutdown(pcb, shut_rx, shut_tx);
}

err_t __wrap_tcp_tcp_get_tcp_addrinfo(struct tcp_pcb * pcb,int local,ip_addr_t * addr,u16_t * port) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __tcp_tcp_get_tcp_addrinfo_req req = { pcb, local, addr, port, &ret };
        __lwip(__tcp_tcp_get_tcp_addrinfo, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_tcp_get_tcp_addrinfo(pcb, local, addr, port);
}

err_t __wrap_tcp_write(struct tcp_pcb * pcb,const void * dataptr,u16_t len,u8_t apiflags) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __tcp_write_req req = { pcb, dataptr, len, apiflags, &ret };
        __lwip(__tcp_write, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_tcp_write(pcb, dataptr, len, apiflags);
}

