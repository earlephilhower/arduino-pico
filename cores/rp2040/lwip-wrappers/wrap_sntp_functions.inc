// Machine generated (makewrapper.py).  DO NOT EDIT
u8_t __wrap_sntp_enabled() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __sntp_enabled_req req = { &ret };
        __lwip(__sntp_enabled, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_sntp_enabled();
}

u8_t __wrap_sntp_getkodreceived(u8_t idx) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __sntp_getkodreceived_req req = { idx, &ret };
        __lwip(__sntp_getkodreceived, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_sntp_getkodreceived(idx);
}

u8_t __wrap_sntp_getoperatingmode() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __sntp_getoperatingmode_req req = { &ret };
        __lwip(__sntp_getoperatingmode, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_sntp_getoperatingmode();
}

u8_t __wrap_sntp_getreachability(u8_t idx) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __sntp_getreachability_req req = { idx, &ret };
        __lwip(__sntp_getreachability, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_sntp_getreachability(idx);
}

const ip_addr_t * __wrap_sntp_getserver(u8_t idx) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        const ip_addr_t * ret;
        __sntp_getserver_req req = { idx, &ret };
        __lwip(__sntp_getserver, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_sntp_getserver(idx);
}

const char * __wrap_sntp_getservername(u8_t idx) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        const char * ret;
        __sntp_getservername_req req = { idx, &ret };
        __lwip(__sntp_getservername, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_sntp_getservername(idx);
}

void __wrap_sntp_init() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sntp_init_req req = {  };
        __lwip(__sntp_init, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sntp_init();
}

#if SNTP_GET_SERVERS_FROM_DHCP || SNTP_GET_SERVERS_FROM_DHCPV6
void __wrap_sntp_servermode_dhcp(int set_servers_from_dhcp) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sntp_servermode_dhcp_req req = { set_servers_from_dhcp };
        __lwip(__sntp_servermode_dhcp, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sntp_servermode_dhcp(set_servers_from_dhcp);
}

#endif
void __wrap_sntp_setoperatingmode(u8_t operating_mode) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sntp_setoperatingmode_req req = { operating_mode };
        __lwip(__sntp_setoperatingmode, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sntp_setoperatingmode(operating_mode);
}

void __wrap_sntp_setserver(u8_t idx,const ip_addr_t * addr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sntp_setserver_req req = { idx, addr };
        __lwip(__sntp_setserver, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sntp_setserver(idx, addr);
}

void __wrap_sntp_setservername(u8_t idx,const char * server) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sntp_setservername_req req = { idx, server };
        __lwip(__sntp_setservername, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sntp_setservername(idx, server);
}

void __wrap_sntp_stop() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sntp_stop_req req = {  };
        __lwip(__sntp_stop, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sntp_stop();
}

