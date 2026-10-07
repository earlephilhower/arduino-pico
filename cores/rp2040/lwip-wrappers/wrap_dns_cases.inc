// Machine generated (makewrapper.py).  DO NOT EDIT
case __dns_gethostbyname: {
    __dns_gethostbyname_req *r = (__dns_gethostbyname_req *)w.req;
    *(r->ret) = __real_dns_gethostbyname(r->hostname, r->addr, r->found, r->callback_arg);
    break;
}
case __dns_gethostbyname_addrtype: {
    __dns_gethostbyname_addrtype_req *r = (__dns_gethostbyname_addrtype_req *)w.req;
    *(r->ret) = __real_dns_gethostbyname_addrtype(r->hostname, r->addr, r->found, r->callback_arg, r->dns_addrtype);
    break;
}
case __dns_getserver: {
    __dns_getserver_req *r = (__dns_getserver_req *)w.req;
    *(r->ret) = __real_dns_getserver(r->numdns);
    break;
}
case __dns_init: {
    __real_dns_init();
    break;
}
#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC
case __dns_local_addhost: {
    __dns_local_addhost_req *r = (__dns_local_addhost_req *)w.req;
    *(r->ret) = __real_dns_local_addhost(r->hostname, r->addr);
    break;
}
#endif
#if DNS_LOCAL_HOSTLIST
case __dns_local_iterate: {
    __dns_local_iterate_req *r = (__dns_local_iterate_req *)w.req;
    *(r->ret) = __real_dns_local_iterate(r->iterator_fn, r->iterator_arg);
    break;
}
#endif
#if DNS_LOCAL_HOSTLIST
case __dns_local_lookup: {
    __dns_local_lookup_req *r = (__dns_local_lookup_req *)w.req;
    *(r->ret) = __real_dns_local_lookup(r->hostname, r->addr, r->dns_addrtype);
    break;
}
#endif
#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC
case __dns_local_removehost: {
    __dns_local_removehost_req *r = (__dns_local_removehost_req *)w.req;
    *(r->ret) = __real_dns_local_removehost(r->hostname, r->addr);
    break;
}
#endif
case __dns_setserver: {
    __dns_setserver_req *r = (__dns_setserver_req *)w.req;
    __real_dns_setserver(r->numdns, r->dnsserver);
    break;
}
case __dns_tmr: {
    __real_dns_tmr();
    break;
}
