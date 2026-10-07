// Machine generated (makewrapper.py).  DO NOT EDIT
err_t __wrap_dns_gethostbyname(const char * hostname,ip_addr_t * addr,dns_found_callback found,void * callback_arg) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __dns_gethostbyname_req req = { hostname, addr, found, callback_arg, &ret };
        __lwip(__dns_gethostbyname, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_dns_gethostbyname(hostname, addr, found, callback_arg);
}

err_t __wrap_dns_gethostbyname_addrtype(const char * hostname,ip_addr_t * addr,dns_found_callback found,void * callback_arg,u8_t dns_addrtype) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __dns_gethostbyname_addrtype_req req = { hostname, addr, found, callback_arg, dns_addrtype, &ret };
        __lwip(__dns_gethostbyname_addrtype, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_dns_gethostbyname_addrtype(hostname, addr, found, callback_arg, dns_addrtype);
}

const ip_addr_t * __wrap_dns_getserver(u8_t numdns) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        const ip_addr_t * ret;
        __dns_getserver_req req = { numdns, &ret };
        __lwip(__dns_getserver, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_dns_getserver(numdns);
}

void __wrap_dns_init() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dns_init_req req = {  };
        __lwip(__dns_init, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dns_init();
}

#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC
err_t __wrap_dns_local_addhost(const char * hostname,const ip_addr_t * addr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __dns_local_addhost_req req = { hostname, addr, &ret };
        __lwip(__dns_local_addhost, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_dns_local_addhost(hostname, addr);
}

#endif
#if DNS_LOCAL_HOSTLIST
size_t __wrap_dns_local_iterate(dns_found_callback iterator_fn,void * iterator_arg) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        size_t ret;
        __dns_local_iterate_req req = { iterator_fn, iterator_arg, &ret };
        __lwip(__dns_local_iterate, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_dns_local_iterate(iterator_fn, iterator_arg);
}

#endif
#if DNS_LOCAL_HOSTLIST
err_t __wrap_dns_local_lookup(const char * hostname,ip_addr_t * addr,u8_t dns_addrtype) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __dns_local_lookup_req req = { hostname, addr, dns_addrtype, &ret };
        __lwip(__dns_local_lookup, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_dns_local_lookup(hostname, addr, dns_addrtype);
}

#endif
#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC
int __wrap_dns_local_removehost(const char * hostname,const ip_addr_t * addr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        int ret;
        __dns_local_removehost_req req = { hostname, addr, &ret };
        __lwip(__dns_local_removehost, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_dns_local_removehost(hostname, addr);
}

#endif
void __wrap_dns_setserver(u8_t numdns,const ip_addr_t * dnsserver) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dns_setserver_req req = { numdns, dnsserver };
        __lwip(__dns_setserver, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dns_setserver(numdns, dnsserver);
}

void __wrap_dns_tmr() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __dns_tmr_req req = {  };
        __lwip(__dns_tmr, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_dns_tmr();
}

