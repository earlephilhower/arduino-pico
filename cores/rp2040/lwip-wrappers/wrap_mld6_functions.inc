// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_IPV6 && LWIP_IPV6_MLD
void __wrap_mld6_input(struct pbuf * p,struct netif * inp) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __mld6_input_req req = { p, inp };
        __lwip(__mld6_input, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_mld6_input(p, inp);
}

#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
err_t __wrap_mld6_joingroup(const ip6_addr_t * srcaddr,const ip6_addr_t * groupaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mld6_joingroup_req req = { srcaddr, groupaddr, &ret };
        __lwip(__mld6_joingroup, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mld6_joingroup(srcaddr, groupaddr);
}

#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
err_t __wrap_mld6_joingroup_netif(struct netif * netif,const ip6_addr_t * groupaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mld6_joingroup_netif_req req = { netif, groupaddr, &ret };
        __lwip(__mld6_joingroup_netif, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mld6_joingroup_netif(netif, groupaddr);
}

#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
err_t __wrap_mld6_leavegroup(const ip6_addr_t * srcaddr,const ip6_addr_t * groupaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mld6_leavegroup_req req = { srcaddr, groupaddr, &ret };
        __lwip(__mld6_leavegroup, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mld6_leavegroup(srcaddr, groupaddr);
}

#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
err_t __wrap_mld6_leavegroup_netif(struct netif * netif,const ip6_addr_t * groupaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mld6_leavegroup_netif_req req = { netif, groupaddr, &ret };
        __lwip(__mld6_leavegroup_netif, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mld6_leavegroup_netif(netif, groupaddr);
}

#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
struct mld_group * __wrap_mld6_lookfor_group(struct netif * ifp,const ip6_addr_t * addr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct mld_group * ret;
        __mld6_lookfor_group_req req = { ifp, addr, &ret };
        __lwip(__mld6_lookfor_group, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mld6_lookfor_group(ifp, addr);
}

#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
void __wrap_mld6_report_groups(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __mld6_report_groups_req req = { netif };
        __lwip(__mld6_report_groups, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_mld6_report_groups(netif);
}

#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
err_t __wrap_mld6_stop(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mld6_stop_req req = { netif, &ret };
        __lwip(__mld6_stop, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mld6_stop(netif);
}

#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
void __wrap_mld6_tmr() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __mld6_tmr_req req = {  };
        __lwip(__mld6_tmr, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_mld6_tmr();
}

#endif
