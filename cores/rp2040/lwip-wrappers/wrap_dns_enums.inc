// Machine generated (makewrapper.py).  DO NOT EDIT
__dns_gethostbyname,
__dns_gethostbyname_addrtype,
__dns_getserver,
__dns_init,
#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC
__dns_local_addhost,
#endif
#if DNS_LOCAL_HOSTLIST
__dns_local_iterate,
#endif
#if DNS_LOCAL_HOSTLIST
__dns_local_lookup,
#endif
#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC
__dns_local_removehost,
#endif
__dns_setserver,
__dns_tmr,
