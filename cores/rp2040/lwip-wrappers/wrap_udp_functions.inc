// Machine generated (makewrapper.py).  DO NOT EDIT
err_t __wrap_udp_bind(struct udp_pcb * pcb,const ip_addr_t * ipaddr,u16_t port) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __udp_bind_req req = { pcb, ipaddr, port, &ret };
        __lwip(__udp_bind, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_bind(pcb, ipaddr, port);
}

void __wrap_udp_bind_netif(struct udp_pcb * pcb,const struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __udp_bind_netif_req req = { pcb, netif };
        __lwip(__udp_bind_netif, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_udp_bind_netif(pcb, netif);
}

err_t __wrap_udp_connect(struct udp_pcb * pcb,const ip_addr_t * ipaddr,u16_t port) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __udp_connect_req req = { pcb, ipaddr, port, &ret };
        __lwip(__udp_connect, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_connect(pcb, ipaddr, port);
}

#if UDP_DEBUG
void __wrap_udp_debug_print(struct udp_hdr * udphdr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __udp_debug_print_req req = { udphdr };
        __lwip(__udp_debug_print, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_udp_debug_print(udphdr);
}

#endif
void __wrap_udp_disconnect(struct udp_pcb * pcb) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __udp_disconnect_req req = { pcb };
        __lwip(__udp_disconnect, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_udp_disconnect(pcb);
}

void __wrap_udp_init() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __udp_init_req req = {  };
        __lwip(__udp_init, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_udp_init();
}

void __wrap_udp_input(struct pbuf * p,struct netif * inp) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __udp_input_req req = { p, inp };
        __lwip(__udp_input, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_udp_input(p, inp);
}

void __wrap_udp_netif_ip_addr_changed(const ip_addr_t * old_addr,const ip_addr_t * new_addr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __udp_netif_ip_addr_changed_req req = { old_addr, new_addr };
        __lwip(__udp_netif_ip_addr_changed, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_udp_netif_ip_addr_changed(old_addr, new_addr);
}

struct udp_pcb * __wrap_udp_new() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct udp_pcb * ret;
        __udp_new_req req = { &ret };
        __lwip(__udp_new, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_new();
}

struct udp_pcb * __wrap_udp_new_ip_type(u8_t type) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct udp_pcb * ret;
        __udp_new_ip_type_req req = { type, &ret };
        __lwip(__udp_new_ip_type, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_new_ip_type(type);
}

void __wrap_udp_recv(struct udp_pcb * pcb,udp_recv_fn recv,void * recv_arg) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __udp_recv_req req = { pcb, recv, recv_arg };
        __lwip(__udp_recv, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_udp_recv(pcb, recv, recv_arg);
}

void __wrap_udp_remove(struct udp_pcb * pcb) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __udp_remove_req req = { pcb };
        __lwip(__udp_remove, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_udp_remove(pcb);
}

err_t __wrap_udp_send(struct udp_pcb * pcb,struct pbuf * p) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __udp_send_req req = { pcb, p, &ret };
        __lwip(__udp_send, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_send(pcb, p);
}

#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
err_t __wrap_udp_send_chksum(struct udp_pcb * pcb,struct pbuf * p,u8_t have_chksum,u16_t chksum) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __udp_send_chksum_req req = { pcb, p, have_chksum, chksum, &ret };
        __lwip(__udp_send_chksum, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_send_chksum(pcb, p, have_chksum, chksum);
}

#endif
err_t __wrap_udp_sendto(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __udp_sendto_req req = { pcb, p, dst_ip, dst_port, &ret };
        __lwip(__udp_sendto, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_sendto(pcb, p, dst_ip, dst_port);
}

#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
err_t __wrap_udp_sendto_chksum(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port,u8_t have_chksum,u16_t chksum) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __udp_sendto_chksum_req req = { pcb, p, dst_ip, dst_port, have_chksum, chksum, &ret };
        __lwip(__udp_sendto_chksum, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_sendto_chksum(pcb, p, dst_ip, dst_port, have_chksum, chksum);
}

#endif
err_t __wrap_udp_sendto_if(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port,struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __udp_sendto_if_req req = { pcb, p, dst_ip, dst_port, netif, &ret };
        __lwip(__udp_sendto_if, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_sendto_if(pcb, p, dst_ip, dst_port, netif);
}

#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
err_t __wrap_udp_sendto_if_chksum(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port,struct netif * netif,u8_t have_chksum,u16_t chksum) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __udp_sendto_if_chksum_req req = { pcb, p, dst_ip, dst_port, netif, have_chksum, chksum, &ret };
        __lwip(__udp_sendto_if_chksum, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_sendto_if_chksum(pcb, p, dst_ip, dst_port, netif, have_chksum, chksum);
}

#endif
err_t __wrap_udp_sendto_if_src(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port,struct netif * netif,const ip_addr_t * src_ip) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __udp_sendto_if_src_req req = { pcb, p, dst_ip, dst_port, netif, src_ip, &ret };
        __lwip(__udp_sendto_if_src, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_sendto_if_src(pcb, p, dst_ip, dst_port, netif, src_ip);
}

#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
err_t __wrap_udp_sendto_if_src_chksum(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port,struct netif * netif,u8_t have_chksum,u16_t chksum,const ip_addr_t * src_ip) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __udp_sendto_if_src_chksum_req req = { pcb, p, dst_ip, dst_port, netif, have_chksum, chksum, src_ip, &ret };
        __lwip(__udp_sendto_if_src_chksum, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_udp_sendto_if_src_chksum(pcb, p, dst_ip, dst_port, netif, have_chksum, chksum, src_ip);
}

#endif
