// Machine generated (makewrapper.py).  DO NOT EDIT
case __netif_add: {
    __netif_add_req *r = (__netif_add_req *)w.req;
    *(r->ret) = __real_netif_add(r->netif, r->ipaddr, r->netmask, r->gw, r->state, r->init, r->input);
    break;
}
#if LWIP_NETIF_EXT_STATUS_CALLBACK
case __netif_add_ext_callback: {
    __netif_add_ext_callback_req *r = (__netif_add_ext_callback_req *)w.req;
    __real_netif_add_ext_callback(r->callback, r->fn);
    break;
}
#endif
#if LWIP_IPV6
case __netif_add_ip6_address: {
    __netif_add_ip6_address_req *r = (__netif_add_ip6_address_req *)w.req;
    *(r->ret) = __real_netif_add_ip6_address(r->netif, r->ip6addr, r->chosen_idx);
    break;
}
#endif
case __netif_add_noaddr: {
    __netif_add_noaddr_req *r = (__netif_add_noaddr_req *)w.req;
    *(r->ret) = __real_netif_add_noaddr(r->netif, r->state, r->init, r->input);
    break;
}
case __netif_alloc_client_data_id: {
    __netif_alloc_client_data_id_req *r = (__netif_alloc_client_data_id_req *)w.req;
    *(r->ret) = __real_netif_alloc_client_data_id();
    break;
}
#if LWIP_IPV6
case __netif_create_ip6_linklocal_address: {
    __netif_create_ip6_linklocal_address_req *r = (__netif_create_ip6_linklocal_address_req *)w.req;
    __real_netif_create_ip6_linklocal_address(r->netif, r->from_mac_48bit);
    break;
}
#endif
case __netif_find: {
    __netif_find_req *r = (__netif_find_req *)w.req;
    *(r->ret) = __real_netif_find(r->name);
    break;
}
case __netif_get_by_index: {
    __netif_get_by_index_req *r = (__netif_get_by_index_req *)w.req;
    *(r->ret) = __real_netif_get_by_index(r->idx);
    break;
}
#if LWIP_IPV6
case __netif_get_ip6_addr_match: {
    __netif_get_ip6_addr_match_req *r = (__netif_get_ip6_addr_match_req *)w.req;
    *(r->ret) = __real_netif_get_ip6_addr_match(r->netif, r->ip6addr);
    break;
}
#endif
#if LWIP_TESTMODE && LWIP_HAVE_LOOPIF
case __netif_get_loopif: {
    __netif_get_loopif_req *r = (__netif_get_loopif_req *)w.req;
    *(r->ret) = __real_netif_get_loopif();
    break;
}
#endif
case __netif_index_to_name: {
    __netif_index_to_name_req *r = (__netif_index_to_name_req *)w.req;
    *(r->ret) = __real_netif_index_to_name(r->idx, r->name);
    break;
}
case __netif_init: {
    __real_netif_init();
    break;
}
case __netif_input: {
    __netif_input_req *r = (__netif_input_req *)w.req;
    *(r->ret) = __real_netif_input(r->p, r->inp);
    break;
}
#if LWIP_NETIF_EXT_STATUS_CALLBACK
case __netif_invoke_ext_callback: {
    __netif_invoke_ext_callback_req *r = (__netif_invoke_ext_callback_req *)w.req;
    __real_netif_invoke_ext_callback(r->netif, r->reason, r->args);
    break;
}
#endif
#if LWIP_IPV6
case __netif_ip6_addr_set: {
    __netif_ip6_addr_set_req *r = (__netif_ip6_addr_set_req *)w.req;
    __real_netif_ip6_addr_set(r->netif, r->addr_idx, r->addr6);
    break;
}
#endif
#if LWIP_IPV6
case __netif_ip6_addr_set_parts: {
    __netif_ip6_addr_set_parts_req *r = (__netif_ip6_addr_set_parts_req *)w.req;
    __real_netif_ip6_addr_set_parts(r->netif, r->addr_idx, r->i0, r->i1, r->i2, r->i3);
    break;
}
#endif
#if LWIP_IPV6
case __netif_ip6_addr_set_state: {
    __netif_ip6_addr_set_state_req *r = (__netif_ip6_addr_set_state_req *)w.req;
    __real_netif_ip6_addr_set_state(r->netif, r->addr_idx, r->state);
    break;
}
#endif
#if ENABLE_LOOPBACK
case __netif_loop_output: {
    __netif_loop_output_req *r = (__netif_loop_output_req *)w.req;
    *(r->ret) = __real_netif_loop_output(r->netif, r->p);
    break;
}
#endif
case __netif_name_to_index: {
    __netif_name_to_index_req *r = (__netif_name_to_index_req *)w.req;
    *(r->ret) = __real_netif_name_to_index(r->name);
    break;
}
#if ENABLE_LOOPBACK
case __netif_poll: {
    __netif_poll_req *r = (__netif_poll_req *)w.req;
    __real_netif_poll(r->netif);
    break;
}
#endif
#if ENABLE_LOOPBACK && !LWIP_NETIF_LOOPBACK_MULTITHREADING
case __netif_poll_all: {
    __real_netif_poll_all();
    break;
}
#endif
case __netif_remove: {
    __netif_remove_req *r = (__netif_remove_req *)w.req;
    __real_netif_remove(r->netif);
    break;
}
#if LWIP_NETIF_EXT_STATUS_CALLBACK
case __netif_remove_ext_callback: {
    __netif_remove_ext_callback_req *r = (__netif_remove_ext_callback_req *)w.req;
    __real_netif_remove_ext_callback(r->callback);
    break;
}
#endif
case __netif_set_addr: {
    __netif_set_addr_req *r = (__netif_set_addr_req *)w.req;
    __real_netif_set_addr(r->netif, r->ipaddr, r->netmask, r->gw);
    break;
}
case __netif_set_default: {
    __netif_set_default_req *r = (__netif_set_default_req *)w.req;
    __real_netif_set_default(r->netif);
    break;
}
case __netif_set_down: {
    __netif_set_down_req *r = (__netif_set_down_req *)w.req;
    __real_netif_set_down(r->netif);
    break;
}
case __netif_set_gw: {
    __netif_set_gw_req *r = (__netif_set_gw_req *)w.req;
    __real_netif_set_gw(r->netif, r->gw);
    break;
}
case __netif_set_ipaddr: {
    __netif_set_ipaddr_req *r = (__netif_set_ipaddr_req *)w.req;
    __real_netif_set_ipaddr(r->netif, r->ipaddr);
    break;
}
case __netif_set_link_callback: {
    __netif_set_link_callback_req *r = (__netif_set_link_callback_req *)w.req;
    __real_netif_set_link_callback(r->netif, r->link_callback);
    break;
}
case __netif_set_link_down: {
    __netif_set_link_down_req *r = (__netif_set_link_down_req *)w.req;
    __real_netif_set_link_down(r->netif);
    break;
}
case __netif_set_link_up: {
    __netif_set_link_up_req *r = (__netif_set_link_up_req *)w.req;
    __real_netif_set_link_up(r->netif);
    break;
}
case __netif_set_netmask: {
    __netif_set_netmask_req *r = (__netif_set_netmask_req *)w.req;
    __real_netif_set_netmask(r->netif, r->netmask);
    break;
}
#if LWIP_NETIF_REMOVE_CALLBACK
case __netif_set_remove_callback: {
    __netif_set_remove_callback_req *r = (__netif_set_remove_callback_req *)w.req;
    __real_netif_set_remove_callback(r->netif, r->remove_callback);
    break;
}
#endif
case __netif_set_status_callback: {
    __netif_set_status_callback_req *r = (__netif_set_status_callback_req *)w.req;
    __real_netif_set_status_callback(r->netif, r->status_callback);
    break;
}
case __netif_set_up: {
    __netif_set_up_req *r = (__netif_set_up_req *)w.req;
    __real_netif_set_up(r->netif);
    break;
}
