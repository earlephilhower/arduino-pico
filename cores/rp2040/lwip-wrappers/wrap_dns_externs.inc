// Machine generated (makewrapper.py).  DO NOT EDIT
extern err_t __real_dns_gethostbyname(const char * hostname,ip_addr_t * addr,dns_found_callback found,void * callback_arg);
extern err_t __real_dns_gethostbyname_addrtype(const char * hostname,ip_addr_t * addr,dns_found_callback found,void * callback_arg,u8_t dns_addrtype);
extern const ip_addr_t * __real_dns_getserver(u8_t numdns);
extern void __real_dns_init();
#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC
extern err_t __real_dns_local_addhost(const char * hostname,const ip_addr_t * addr);
#endif
#if DNS_LOCAL_HOSTLIST
extern size_t __real_dns_local_iterate(dns_found_callback iterator_fn,void * iterator_arg);
#endif
#if DNS_LOCAL_HOSTLIST
extern err_t __real_dns_local_lookup(const char * hostname,ip_addr_t * addr,u8_t dns_addrtype);
#endif
#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC
extern int __real_dns_local_removehost(const char * hostname,const ip_addr_t * addr);
#endif
extern void __real_dns_setserver(u8_t numdns,const ip_addr_t * dnsserver);
extern void __real_dns_tmr();
