// Machine generated (makewrapper.py).  DO NOT EDIT
err_t __wrap_raw_bind(struct raw_pcb * pcb,const ip_addr_t * ipaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __raw_bind_req req = { pcb, ipaddr, &ret };
        __lwip(__raw_bind, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_raw_bind(pcb, ipaddr);
}

void __wrap_raw_bind_netif(struct raw_pcb * pcb,const struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __raw_bind_netif_req req = { pcb, netif };
        __lwip(__raw_bind_netif, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_raw_bind_netif(pcb, netif);
}

err_t __wrap_raw_connect(struct raw_pcb * pcb,const ip_addr_t * ipaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __raw_connect_req req = { pcb, ipaddr, &ret };
        __lwip(__raw_connect, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_raw_connect(pcb, ipaddr);
}

void __wrap_raw_disconnect(struct raw_pcb * pcb) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __raw_disconnect_req req = { pcb };
        __lwip(__raw_disconnect, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_raw_disconnect(pcb);
}

struct raw_pcb * __wrap_raw_new(u8_t proto) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct raw_pcb * ret;
        __raw_new_req req = { proto, &ret };
        __lwip(__raw_new, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_raw_new(proto);
}

struct raw_pcb * __wrap_raw_new_ip_type(u8_t type,u8_t proto) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct raw_pcb * ret;
        __raw_new_ip_type_req req = { type, proto, &ret };
        __lwip(__raw_new_ip_type, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_raw_new_ip_type(type, proto);
}

void __wrap_raw_recv(struct raw_pcb * pcb,raw_recv_fn recv,void * recv_arg) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __raw_recv_req req = { pcb, recv, recv_arg };
        __lwip(__raw_recv, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_raw_recv(pcb, recv, recv_arg);
}

void __wrap_raw_remove(struct raw_pcb * pcb) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __raw_remove_req req = { pcb };
        __lwip(__raw_remove, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_raw_remove(pcb);
}

err_t __wrap_raw_send(struct raw_pcb * pcb,struct pbuf * p) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __raw_send_req req = { pcb, p, &ret };
        __lwip(__raw_send, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_raw_send(pcb, p);
}

err_t __wrap_raw_sendto(struct raw_pcb * pcb,struct pbuf * p,const ip_addr_t * ipaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __raw_sendto_req req = { pcb, p, ipaddr, &ret };
        __lwip(__raw_sendto, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_raw_sendto(pcb, p, ipaddr);
}

err_t __wrap_raw_sendto_if_src(struct raw_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,struct netif * netif,const ip_addr_t * src_ip) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __raw_sendto_if_src_req req = { pcb, p, dst_ip, netif, src_ip, &ret };
        __lwip(__raw_sendto_if_src, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_raw_sendto_if_src(pcb, p, dst_ip, netif, src_ip);
}

