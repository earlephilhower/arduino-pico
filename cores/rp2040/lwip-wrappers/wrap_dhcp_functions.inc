// Machine generated (makewrapper.py).  DO NOT EDIT
void __wrap_dhcp_cleanup(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dhcp_cleanup_req req = { netif };
        __lwip(__dhcp_cleanup, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dhcp_cleanup(netif);
}

void __wrap_dhcp_coarse_tmr() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dhcp_coarse_tmr_req req = {  };
        __lwip(__dhcp_coarse_tmr, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dhcp_coarse_tmr();
}

void __wrap_dhcp_fine_tmr() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dhcp_fine_tmr_req req = {  };
        __lwip(__dhcp_fine_tmr, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dhcp_fine_tmr();
}

void __wrap_dhcp_inform(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dhcp_inform_req req = { netif };
        __lwip(__dhcp_inform, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dhcp_inform(netif);
}

void __wrap_dhcp_network_changed_link_up(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dhcp_network_changed_link_up_req req = { netif };
        __lwip(__dhcp_network_changed_link_up, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dhcp_network_changed_link_up(netif);
}

err_t __wrap_dhcp_release(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __dhcp_release_req req = { netif, &ret };
        __lwip(__dhcp_release, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_dhcp_release(netif);
}

void __wrap_dhcp_release_and_stop(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dhcp_release_and_stop_req req = { netif };
        __lwip(__dhcp_release_and_stop, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dhcp_release_and_stop(netif);
}

err_t __wrap_dhcp_renew(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __dhcp_renew_req req = { netif, &ret };
        __lwip(__dhcp_renew, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_dhcp_renew(netif);
}

#if LWIP_DHCP && SNTP_GET_SERVERS_FROM_DHCP
void __wrap_dhcp_set_ntp_servers(u8_t num_ntp_servers,const ip4_addr_t * ntp_server_addrs) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dhcp_set_ntp_servers_req req = { num_ntp_servers, ntp_server_addrs };
        __lwip(__dhcp_set_ntp_servers, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dhcp_set_ntp_servers(num_ntp_servers, ntp_server_addrs);
}

#endif
void __wrap_dhcp_set_struct(struct netif * netif,struct dhcp * dhcp) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dhcp_set_struct_req req = { netif, dhcp };
        __lwip(__dhcp_set_struct, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dhcp_set_struct(netif, dhcp);
}

err_t __wrap_dhcp_start(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __dhcp_start_req req = { netif, &ret };
        __lwip(__dhcp_start, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_dhcp_start(netif);
}

void __wrap_dhcp_stop(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dhcp_stop_req req = { netif };
        __lwip(__dhcp_stop, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dhcp_stop(netif);
}

u8_t __wrap_dhcp_supplied_address(const struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __dhcp_supplied_address_req req = { netif, &ret };
        __lwip(__dhcp_supplied_address, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_dhcp_supplied_address(netif);
}

