// Machine generated (makewrapper.py).  DO NOT EDIT
case __mdns_get_service_txt_userdata: {
    __mdns_get_service_txt_userdata_req *r = (__mdns_get_service_txt_userdata_req *)w.req;
    *(r->ret) = __real_mdns_get_service_txt_userdata(r->netif, r->slot);
    break;
}
case __mdns_resp_add_netif: {
    __mdns_resp_add_netif_req *r = (__mdns_resp_add_netif_req *)w.req;
    *(r->ret) = __real_mdns_resp_add_netif(r->netif, r->hostname);
    break;
}
case __mdns_resp_add_service: {
    __mdns_resp_add_service_req *r = (__mdns_resp_add_service_req *)w.req;
    *(r->ret) = __real_mdns_resp_add_service(r->netif, r->name, r->service, r->proto, r->port, r->txt_fn, r->txt_userdata);
    break;
}
case __mdns_resp_add_service_txtitem: {
    __mdns_resp_add_service_txtitem_req *r = (__mdns_resp_add_service_txtitem_req *)w.req;
    *(r->ret) = __real_mdns_resp_add_service_txtitem(r->service, r->txt, r->txt_len);
    break;
}
case __mdns_resp_announce: {
    __mdns_resp_announce_req *r = (__mdns_resp_announce_req *)w.req;
    __real_mdns_resp_announce(r->netif);
    break;
}
case __mdns_resp_del_service: {
    __mdns_resp_del_service_req *r = (__mdns_resp_del_service_req *)w.req;
    *(r->ret) = __real_mdns_resp_del_service(r->netif, r->slot);
    break;
}
case __mdns_resp_init: {
    __real_mdns_resp_init();
    break;
}
case __mdns_resp_netif_active: {
    __mdns_resp_netif_active_req *r = (__mdns_resp_netif_active_req *)w.req;
    *(r->ret) = __real_mdns_resp_netif_active(r->netif);
    break;
}
case __mdns_resp_register_name_result_cb: {
    __mdns_resp_register_name_result_cb_req *r = (__mdns_resp_register_name_result_cb_req *)w.req;
    __real_mdns_resp_register_name_result_cb(r->cb);
    break;
}
case __mdns_resp_remove_netif: {
    __mdns_resp_remove_netif_req *r = (__mdns_resp_remove_netif_req *)w.req;
    *(r->ret) = __real_mdns_resp_remove_netif(r->netif);
    break;
}
case __mdns_resp_rename_netif: {
    __mdns_resp_rename_netif_req *r = (__mdns_resp_rename_netif_req *)w.req;
    *(r->ret) = __real_mdns_resp_rename_netif(r->netif, r->hostname);
    break;
}
case __mdns_resp_rename_service: {
    __mdns_resp_rename_service_req *r = (__mdns_resp_rename_service_req *)w.req;
    *(r->ret) = __real_mdns_resp_rename_service(r->netif, r->slot, r->name);
    break;
}
case __mdns_resp_restart: {
    __mdns_resp_restart_req *r = (__mdns_resp_restart_req *)w.req;
    __real_mdns_resp_restart(r->netif);
    break;
}
case __mdns_resp_restart_delay: {
    __mdns_resp_restart_delay_req *r = (__mdns_resp_restart_delay_req *)w.req;
    __real_mdns_resp_restart_delay(r->netif, r->delay);
    break;
}
case __mdns_search_service: {
    __mdns_search_service_req *r = (__mdns_search_service_req *)w.req;
    *(r->ret) = __real_mdns_search_service(r->name, r->service, r->proto, r->netif, r->result_fn, r->arg, r->request_id);
    break;
}
case __mdns_search_stop: {
    __mdns_search_stop_req *r = (__mdns_search_stop_req *)w.req;
    __real_mdns_search_stop(r->request_id);
    break;
}
