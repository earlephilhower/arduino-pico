// Machine generated (makewrapper.py).  DO NOT EDIT
case __dhcp_cleanup: {
    __dhcp_cleanup_req *r = (__dhcp_cleanup_req *)w.req;
    __real_dhcp_cleanup(r->netif);
    break;
}
case __dhcp_coarse_tmr: {
    __real_dhcp_coarse_tmr();
    break;
}
case __dhcp_fine_tmr: {
    __real_dhcp_fine_tmr();
    break;
}
case __dhcp_inform: {
    __dhcp_inform_req *r = (__dhcp_inform_req *)w.req;
    __real_dhcp_inform(r->netif);
    break;
}
case __dhcp_network_changed_link_up: {
    __dhcp_network_changed_link_up_req *r = (__dhcp_network_changed_link_up_req *)w.req;
    __real_dhcp_network_changed_link_up(r->netif);
    break;
}
case __dhcp_release: {
    __dhcp_release_req *r = (__dhcp_release_req *)w.req;
    *(r->ret) = __real_dhcp_release(r->netif);
    break;
}
case __dhcp_release_and_stop: {
    __dhcp_release_and_stop_req *r = (__dhcp_release_and_stop_req *)w.req;
    __real_dhcp_release_and_stop(r->netif);
    break;
}
case __dhcp_renew: {
    __dhcp_renew_req *r = (__dhcp_renew_req *)w.req;
    *(r->ret) = __real_dhcp_renew(r->netif);
    break;
}
#if LWIP_DHCP && SNTP_GET_SERVERS_FROM_DHCP
case __dhcp_set_ntp_servers: {
    __dhcp_set_ntp_servers_req *r = (__dhcp_set_ntp_servers_req *)w.req;
    __real_dhcp_set_ntp_servers(r->num_ntp_servers, r->ntp_server_addrs);
    break;
}
#endif
case __dhcp_set_struct: {
    __dhcp_set_struct_req *r = (__dhcp_set_struct_req *)w.req;
    __real_dhcp_set_struct(r->netif, r->dhcp);
    break;
}
case __dhcp_start: {
    __dhcp_start_req *r = (__dhcp_start_req *)w.req;
    *(r->ret) = __real_dhcp_start(r->netif);
    break;
}
case __dhcp_stop: {
    __dhcp_stop_req *r = (__dhcp_stop_req *)w.req;
    __real_dhcp_stop(r->netif);
    break;
}
case __dhcp_supplied_address: {
    __dhcp_supplied_address_req *r = (__dhcp_supplied_address_req *)w.req;
    *(r->ret) = __real_dhcp_supplied_address(r->netif);
    break;
}
