// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_IPV6 && LWIP_IPV6_MLD
case __mld6_input: {
    __mld6_input_req *r = (__mld6_input_req *)w.req;
    __real_mld6_input(r->p, r->inp);
    break;
}
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
case __mld6_joingroup: {
    __mld6_joingroup_req *r = (__mld6_joingroup_req *)w.req;
    *(r->ret) = __real_mld6_joingroup(r->srcaddr, r->groupaddr);
    break;
}
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
case __mld6_joingroup_netif: {
    __mld6_joingroup_netif_req *r = (__mld6_joingroup_netif_req *)w.req;
    *(r->ret) = __real_mld6_joingroup_netif(r->netif, r->groupaddr);
    break;
}
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
case __mld6_leavegroup: {
    __mld6_leavegroup_req *r = (__mld6_leavegroup_req *)w.req;
    *(r->ret) = __real_mld6_leavegroup(r->srcaddr, r->groupaddr);
    break;
}
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
case __mld6_leavegroup_netif: {
    __mld6_leavegroup_netif_req *r = (__mld6_leavegroup_netif_req *)w.req;
    *(r->ret) = __real_mld6_leavegroup_netif(r->netif, r->groupaddr);
    break;
}
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
case __mld6_lookfor_group: {
    __mld6_lookfor_group_req *r = (__mld6_lookfor_group_req *)w.req;
    *(r->ret) = __real_mld6_lookfor_group(r->ifp, r->addr);
    break;
}
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
case __mld6_report_groups: {
    __mld6_report_groups_req *r = (__mld6_report_groups_req *)w.req;
    __real_mld6_report_groups(r->netif);
    break;
}
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
case __mld6_stop: {
    __mld6_stop_req *r = (__mld6_stop_req *)w.req;
    *(r->ret) = __real_mld6_stop(r->netif);
    break;
}
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
case __mld6_tmr: {
    __real_mld6_tmr();
    break;
}
#endif
