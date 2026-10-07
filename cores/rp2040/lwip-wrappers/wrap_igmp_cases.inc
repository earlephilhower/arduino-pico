// Machine generated (makewrapper.py).  DO NOT EDIT
case __igmp_init: {
    __real_igmp_init();
    break;
}
case __igmp_input: {
    __igmp_input_req *r = (__igmp_input_req *)w.req;
    __real_igmp_input(r->p, r->inp, r->dest);
    break;
}
case __igmp_joingroup: {
    __igmp_joingroup_req *r = (__igmp_joingroup_req *)w.req;
    *(r->ret) = __real_igmp_joingroup(r->ifaddr, r->groupaddr);
    break;
}
case __igmp_joingroup_netif: {
    __igmp_joingroup_netif_req *r = (__igmp_joingroup_netif_req *)w.req;
    *(r->ret) = __real_igmp_joingroup_netif(r->netif, r->groupaddr);
    break;
}
case __igmp_leavegroup: {
    __igmp_leavegroup_req *r = (__igmp_leavegroup_req *)w.req;
    *(r->ret) = __real_igmp_leavegroup(r->ifaddr, r->groupaddr);
    break;
}
case __igmp_leavegroup_netif: {
    __igmp_leavegroup_netif_req *r = (__igmp_leavegroup_netif_req *)w.req;
    *(r->ret) = __real_igmp_leavegroup_netif(r->netif, r->groupaddr);
    break;
}
case __igmp_lookfor_group: {
    __igmp_lookfor_group_req *r = (__igmp_lookfor_group_req *)w.req;
    *(r->ret) = __real_igmp_lookfor_group(r->ifp, r->addr);
    break;
}
case __igmp_report_groups: {
    __igmp_report_groups_req *r = (__igmp_report_groups_req *)w.req;
    __real_igmp_report_groups(r->netif);
    break;
}
case __igmp_start: {
    __igmp_start_req *r = (__igmp_start_req *)w.req;
    *(r->ret) = __real_igmp_start(r->netif);
    break;
}
case __igmp_stop: {
    __igmp_stop_req *r = (__igmp_stop_req *)w.req;
    *(r->ret) = __real_igmp_stop(r->netif);
    break;
}
case __igmp_tmr: {
    __real_igmp_tmr();
    break;
}
