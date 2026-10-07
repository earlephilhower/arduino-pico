// Machine generated (makewrapper.py).  DO NOT EDIT
case __sntp_enabled: {
    __sntp_enabled_req *r = (__sntp_enabled_req *)w.req;
    *(r->ret) = __real_sntp_enabled();
    break;
}
case __sntp_getkodreceived: {
    __sntp_getkodreceived_req *r = (__sntp_getkodreceived_req *)w.req;
    *(r->ret) = __real_sntp_getkodreceived(r->idx);
    break;
}
case __sntp_getoperatingmode: {
    __sntp_getoperatingmode_req *r = (__sntp_getoperatingmode_req *)w.req;
    *(r->ret) = __real_sntp_getoperatingmode();
    break;
}
case __sntp_getreachability: {
    __sntp_getreachability_req *r = (__sntp_getreachability_req *)w.req;
    *(r->ret) = __real_sntp_getreachability(r->idx);
    break;
}
case __sntp_getserver: {
    __sntp_getserver_req *r = (__sntp_getserver_req *)w.req;
    *(r->ret) = __real_sntp_getserver(r->idx);
    break;
}
case __sntp_getservername: {
    __sntp_getservername_req *r = (__sntp_getservername_req *)w.req;
    *(r->ret) = __real_sntp_getservername(r->idx);
    break;
}
case __sntp_init: {
    __real_sntp_init();
    break;
}
#if SNTP_GET_SERVERS_FROM_DHCP || SNTP_GET_SERVERS_FROM_DHCPV6
case __sntp_servermode_dhcp: {
    __sntp_servermode_dhcp_req *r = (__sntp_servermode_dhcp_req *)w.req;
    __real_sntp_servermode_dhcp(r->set_servers_from_dhcp);
    break;
}
#endif
case __sntp_setoperatingmode: {
    __sntp_setoperatingmode_req *r = (__sntp_setoperatingmode_req *)w.req;
    __real_sntp_setoperatingmode(r->operating_mode);
    break;
}
case __sntp_setserver: {
    __sntp_setserver_req *r = (__sntp_setserver_req *)w.req;
    __real_sntp_setserver(r->idx, r->addr);
    break;
}
case __sntp_setservername: {
    __sntp_setservername_req *r = (__sntp_setservername_req *)w.req;
    __real_sntp_setservername(r->idx, r->server);
    break;
}
case __sntp_stop: {
    __real_sntp_stop();
    break;
}
