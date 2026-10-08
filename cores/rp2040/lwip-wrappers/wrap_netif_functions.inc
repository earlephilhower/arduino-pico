// Machine generated (makewrapper.py).  DO NOT EDIT
struct netif * __wrap_netif_add(struct netif * netif,const ip4_addr_t * ipaddr,const ip4_addr_t * netmask,const ip4_addr_t * gw,void * state,netif_init_fn init,netif_input_fn input) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct netif * ret;
        __netif_add_req req = { netif, ipaddr, netmask, gw, state, init, input, &ret };
        __lwip(__netif_add, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_add(netif, ipaddr, netmask, gw, state, init, input);
}

#if LWIP_NETIF_EXT_STATUS_CALLBACK
void __wrap_netif_add_ext_callback(netif_ext_callback_t * callback,netif_ext_callback_fn fn) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_add_ext_callback_req req = { callback, fn };
        __lwip(__netif_add_ext_callback, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_add_ext_callback(callback, fn);
}

#endif
#if LWIP_IPV6
err_t __wrap_netif_add_ip6_address(struct netif * netif,const ip6_addr_t * ip6addr,s8_t * chosen_idx) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __netif_add_ip6_address_req req = { netif, ip6addr, chosen_idx, &ret };
        __lwip(__netif_add_ip6_address, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_add_ip6_address(netif, ip6addr, chosen_idx);
}

#endif
struct netif * __wrap_netif_add_noaddr(struct netif * netif,void * state,netif_init_fn init,netif_input_fn input) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct netif * ret;
        __netif_add_noaddr_req req = { netif, state, init, input, &ret };
        __lwip(__netif_add_noaddr, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_add_noaddr(netif, state, init, input);
}

u8_t __wrap_netif_alloc_client_data_id() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __netif_alloc_client_data_id_req req = { &ret };
        __lwip(__netif_alloc_client_data_id, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_alloc_client_data_id();
}

#if LWIP_IPV6
void __wrap_netif_create_ip6_linklocal_address(struct netif * netif,u8_t from_mac_48bit) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_create_ip6_linklocal_address_req req = { netif, from_mac_48bit };
        __lwip(__netif_create_ip6_linklocal_address, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_create_ip6_linklocal_address(netif, from_mac_48bit);
}

#endif
struct netif * __wrap_netif_find(const char * name) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct netif * ret;
        __netif_find_req req = { name, &ret };
        __lwip(__netif_find, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_find(name);
}

struct netif * __wrap_netif_get_by_index(u8_t idx) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct netif * ret;
        __netif_get_by_index_req req = { idx, &ret };
        __lwip(__netif_get_by_index, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_get_by_index(idx);
}

#if LWIP_IPV6
s8_t __wrap_netif_get_ip6_addr_match(struct netif * netif,const ip6_addr_t * ip6addr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        s8_t ret;
        __netif_get_ip6_addr_match_req req = { netif, ip6addr, &ret };
        __lwip(__netif_get_ip6_addr_match, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_get_ip6_addr_match(netif, ip6addr);
}

#endif
#if LWIP_TESTMODE && LWIP_HAVE_LOOPIF
struct netif * __wrap_netif_get_loopif() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct netif * ret;
        __netif_get_loopif_req req = { &ret };
        __lwip(__netif_get_loopif, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_get_loopif();
}

#endif
char * __wrap_netif_index_to_name(u8_t idx,char * name) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        char * ret;
        __netif_index_to_name_req req = { idx, name, &ret };
        __lwip(__netif_index_to_name, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_index_to_name(idx, name);
}

void __wrap_netif_init() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_init_req req = {  };
        __lwip(__netif_init, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_init();
}

err_t __wrap_netif_input(struct pbuf * p,struct netif * inp) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __netif_input_req req = { p, inp, &ret };
        __lwip(__netif_input, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_input(p, inp);
}

#if LWIP_NETIF_EXT_STATUS_CALLBACK
void __wrap_netif_invoke_ext_callback(struct netif * netif,netif_nsc_reason_t reason,const netif_ext_callback_args_t * args) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_invoke_ext_callback_req req = { netif, reason, args };
        __lwip(__netif_invoke_ext_callback, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_invoke_ext_callback(netif, reason, args);
}

#endif
#if LWIP_IPV6
void __wrap_netif_ip6_addr_set(struct netif * netif,s8_t addr_idx,const ip6_addr_t * addr6) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_ip6_addr_set_req req = { netif, addr_idx, addr6 };
        __lwip(__netif_ip6_addr_set, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_ip6_addr_set(netif, addr_idx, addr6);
}

#endif
#if LWIP_IPV6
void __wrap_netif_ip6_addr_set_parts(struct netif * netif,s8_t addr_idx,u32_t i0,u32_t i1,u32_t i2,u32_t i3) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_ip6_addr_set_parts_req req = { netif, addr_idx, i0, i1, i2, i3 };
        __lwip(__netif_ip6_addr_set_parts, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_ip6_addr_set_parts(netif, addr_idx, i0, i1, i2, i3);
}

#endif
#if LWIP_IPV6
void __wrap_netif_ip6_addr_set_state(struct netif * netif,s8_t addr_idx,u8_t state) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_ip6_addr_set_state_req req = { netif, addr_idx, state };
        __lwip(__netif_ip6_addr_set_state, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_ip6_addr_set_state(netif, addr_idx, state);
}

#endif
#if ENABLE_LOOPBACK
err_t __wrap_netif_loop_output(struct netif * netif,struct pbuf * p) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __netif_loop_output_req req = { netif, p, &ret };
        __lwip(__netif_loop_output, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_loop_output(netif, p);
}

#endif
u8_t __wrap_netif_name_to_index(const char * name) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __netif_name_to_index_req req = { name, &ret };
        __lwip(__netif_name_to_index, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_netif_name_to_index(name);
}

#if ENABLE_LOOPBACK
void __wrap_netif_poll(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_poll_req req = { netif };
        __lwip(__netif_poll, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_poll(netif);
}

#endif
#if ENABLE_LOOPBACK && !LWIP_NETIF_LOOPBACK_MULTITHREADING
void __wrap_netif_poll_all() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_poll_all_req req = {  };
        __lwip(__netif_poll_all, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_poll_all();
}

#endif
void __wrap_netif_remove(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_remove_req req = { netif };
        __lwip(__netif_remove, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_remove(netif);
}

#if LWIP_NETIF_EXT_STATUS_CALLBACK
void __wrap_netif_remove_ext_callback(netif_ext_callback_t * callback) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_remove_ext_callback_req req = { callback };
        __lwip(__netif_remove_ext_callback, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_remove_ext_callback(callback);
}

#endif
void __wrap_netif_set_addr(struct netif * netif,const ip4_addr_t * ipaddr,const ip4_addr_t * netmask,const ip4_addr_t * gw) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_addr_req req = { netif, ipaddr, netmask, gw };
        __lwip(__netif_set_addr, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_addr(netif, ipaddr, netmask, gw);
}

void __wrap_netif_set_default(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_default_req req = { netif };
        __lwip(__netif_set_default, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_default(netif);
}

void __wrap_netif_set_down(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_down_req req = { netif };
        __lwip(__netif_set_down, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_down(netif);
}

void __wrap_netif_set_gw(struct netif * netif,const ip4_addr_t * gw) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_gw_req req = { netif, gw };
        __lwip(__netif_set_gw, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_gw(netif, gw);
}

void __wrap_netif_set_ipaddr(struct netif * netif,const ip4_addr_t * ipaddr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_ipaddr_req req = { netif, ipaddr };
        __lwip(__netif_set_ipaddr, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_ipaddr(netif, ipaddr);
}

void __wrap_netif_set_link_callback(struct netif * netif,netif_status_callback_fn link_callback) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_link_callback_req req = { netif, link_callback };
        __lwip(__netif_set_link_callback, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_link_callback(netif, link_callback);
}

void __wrap_netif_set_link_down(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_link_down_req req = { netif };
        __lwip(__netif_set_link_down, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_link_down(netif);
}

void __wrap_netif_set_link_up(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_link_up_req req = { netif };
        __lwip(__netif_set_link_up, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_link_up(netif);
}

void __wrap_netif_set_netmask(struct netif * netif,const ip4_addr_t * netmask) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_netmask_req req = { netif, netmask };
        __lwip(__netif_set_netmask, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_netmask(netif, netmask);
}

#if LWIP_NETIF_REMOVE_CALLBACK
void __wrap_netif_set_remove_callback(struct netif * netif,netif_status_callback_fn remove_callback) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_remove_callback_req req = { netif, remove_callback };
        __lwip(__netif_set_remove_callback, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_remove_callback(netif, remove_callback);
}

#endif
void __wrap_netif_set_status_callback(struct netif * netif,netif_status_callback_fn status_callback) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_status_callback_req req = { netif, status_callback };
        __lwip(__netif_set_status_callback, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_status_callback(netif, status_callback);
}

void __wrap_netif_set_up(struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __netif_set_up_req req = { netif };
        __lwip(__netif_set_up, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_netif_set_up(netif);
}

