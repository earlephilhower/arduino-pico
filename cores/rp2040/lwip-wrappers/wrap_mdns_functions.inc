// Machine generated (makewrapper.py).  DO NOT EDIT
void * __wrap_mdns_get_service_txt_userdata(struct netif * netif,s8_t slot) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        void * ret;
        __mdns_get_service_txt_userdata_req req = { netif, slot, &ret };
        __lwip(__mdns_get_service_txt_userdata, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mdns_get_service_txt_userdata(netif, slot);
}

err_t __wrap_mdns_resp_add_netif(struct netif * netif,const char * hostname) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mdns_resp_add_netif_req req = { netif, hostname, &ret };
        __lwip(__mdns_resp_add_netif, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mdns_resp_add_netif(netif, hostname);
}

s8_t __wrap_mdns_resp_add_service(struct netif * netif,const char * name,const char * service,enum mdns_sd_proto proto,u16_t port,service_get_txt_fn_t txt_fn,void * txt_userdata) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        s8_t ret;
        __mdns_resp_add_service_req req = { netif, name, service, proto, port, txt_fn, txt_userdata, &ret };
        __lwip(__mdns_resp_add_service, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mdns_resp_add_service(netif, name, service, proto, port, txt_fn, txt_userdata);
}

err_t __wrap_mdns_resp_add_service_txtitem(struct mdns_service * service,const char * txt,u8_t txt_len) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mdns_resp_add_service_txtitem_req req = { service, txt, txt_len, &ret };
        __lwip(__mdns_resp_add_service_txtitem, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mdns_resp_add_service_txtitem(service, txt, txt_len);
}

void __wrap_mdns_resp_announce(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __mdns_resp_announce_req req = { netif };
        __lwip(__mdns_resp_announce, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_mdns_resp_announce(netif);
}

err_t __wrap_mdns_resp_del_service(struct netif * netif,u8_t slot) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mdns_resp_del_service_req req = { netif, slot, &ret };
        __lwip(__mdns_resp_del_service, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mdns_resp_del_service(netif, slot);
}

void __wrap_mdns_resp_init() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __mdns_resp_init_req req = {  };
        __lwip(__mdns_resp_init, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_mdns_resp_init();
}

int __wrap_mdns_resp_netif_active(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        int ret;
        __mdns_resp_netif_active_req req = { netif, &ret };
        __lwip(__mdns_resp_netif_active, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mdns_resp_netif_active(netif);
}

void __wrap_mdns_resp_register_name_result_cb(mdns_name_result_cb_t cb) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __mdns_resp_register_name_result_cb_req req = { cb };
        __lwip(__mdns_resp_register_name_result_cb, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_mdns_resp_register_name_result_cb(cb);
}

err_t __wrap_mdns_resp_remove_netif(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mdns_resp_remove_netif_req req = { netif, &ret };
        __lwip(__mdns_resp_remove_netif, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mdns_resp_remove_netif(netif);
}

err_t __wrap_mdns_resp_rename_netif(struct netif * netif,const char * hostname) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mdns_resp_rename_netif_req req = { netif, hostname, &ret };
        __lwip(__mdns_resp_rename_netif, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mdns_resp_rename_netif(netif, hostname);
}

err_t __wrap_mdns_resp_rename_service(struct netif * netif,u8_t slot,const char * name) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mdns_resp_rename_service_req req = { netif, slot, name, &ret };
        __lwip(__mdns_resp_rename_service, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mdns_resp_rename_service(netif, slot, name);
}

void __wrap_mdns_resp_restart(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __mdns_resp_restart_req req = { netif };
        __lwip(__mdns_resp_restart, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_mdns_resp_restart(netif);
}

void __wrap_mdns_resp_restart_delay(struct netif * netif,uint32_t delay) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __mdns_resp_restart_delay_req req = { netif, delay };
        __lwip(__mdns_resp_restart_delay, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_mdns_resp_restart_delay(netif, delay);
}

err_t __wrap_mdns_search_service(const char * name,const char * service,enum mdns_sd_proto proto,struct netif * netif,search_result_fn_t result_fn,void * arg,u8_t * request_id) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __mdns_search_service_req req = { name, service, proto, netif, result_fn, arg, request_id, &ret };
        __lwip(__mdns_search_service, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_mdns_search_service(name, service, proto, netif, result_fn, arg, request_id);
}

void __wrap_mdns_search_stop(u8_t request_id) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __mdns_search_stop_req req = { request_id };
        __lwip(__mdns_search_stop, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_mdns_search_stop(request_id);
}

