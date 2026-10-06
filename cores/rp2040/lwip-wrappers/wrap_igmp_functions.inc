// Machine generated (makewrapper.py).  DO NOT EDIT
void __wrap_igmp_init() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __igmp_init_req req = {  };
        __lwip(__igmp_init, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_igmp_init();
}

void __wrap_igmp_input(struct pbuf * p,struct netif * inp,const ip4_addr_t * dest) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __igmp_input_req req = { p, inp, dest };
        __lwip(__igmp_input, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_igmp_input(p, inp, dest);
}

err_t __wrap_igmp_joingroup(const ip4_addr_t * ifaddr,const ip4_addr_t * groupaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __igmp_joingroup_req req = { ifaddr, groupaddr, &ret };
        __lwip(__igmp_joingroup, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_igmp_joingroup(ifaddr, groupaddr);
}

err_t __wrap_igmp_joingroup_netif(struct netif * netif,const ip4_addr_t * groupaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __igmp_joingroup_netif_req req = { netif, groupaddr, &ret };
        __lwip(__igmp_joingroup_netif, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_igmp_joingroup_netif(netif, groupaddr);
}

err_t __wrap_igmp_leavegroup(const ip4_addr_t * ifaddr,const ip4_addr_t * groupaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __igmp_leavegroup_req req = { ifaddr, groupaddr, &ret };
        __lwip(__igmp_leavegroup, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_igmp_leavegroup(ifaddr, groupaddr);
}

err_t __wrap_igmp_leavegroup_netif(struct netif * netif,const ip4_addr_t * groupaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __igmp_leavegroup_netif_req req = { netif, groupaddr, &ret };
        __lwip(__igmp_leavegroup_netif, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_igmp_leavegroup_netif(netif, groupaddr);
}

struct igmp_group * __wrap_igmp_lookfor_group(struct netif * ifp,const ip4_addr_t * addr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct igmp_group * ret;
        __igmp_lookfor_group_req req = { ifp, addr, &ret };
        __lwip(__igmp_lookfor_group, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_igmp_lookfor_group(ifp, addr);
}

void __wrap_igmp_report_groups(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __igmp_report_groups_req req = { netif };
        __lwip(__igmp_report_groups, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_igmp_report_groups(netif);
}

err_t __wrap_igmp_start(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __igmp_start_req req = { netif, &ret };
        __lwip(__igmp_start, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_igmp_start(netif);
}

err_t __wrap_igmp_stop(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __igmp_stop_req req = { netif, &ret };
        __lwip(__igmp_stop, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_igmp_stop(netif);
}

void __wrap_igmp_tmr() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __igmp_tmr_req req = {  };
        __lwip(__igmp_tmr, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_igmp_tmr();
}

