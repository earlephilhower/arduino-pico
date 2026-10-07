// Machine generated (makewrapper.py).  DO NOT EDIT
typedef struct {
    const char * hostname;
    ip_addr_t * addr;
    dns_found_callback found;
    void * callback_arg;
    err_t *ret;
} __dns_gethostbyname_req;
typedef struct {
    const char * hostname;
    ip_addr_t * addr;
    dns_found_callback found;
    void * callback_arg;
    u8_t dns_addrtype;
    err_t *ret;
} __dns_gethostbyname_addrtype_req;
typedef struct {
    u8_t numdns;
    const ip_addr_t * *ret;
} __dns_getserver_req;
typedef struct {
    ;
} __dns_init_req;
#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC
typedef struct {
    const char * hostname;
    const ip_addr_t * addr;
    err_t *ret;
} __dns_local_addhost_req;
#endif
#if DNS_LOCAL_HOSTLIST
typedef struct {
    dns_found_callback iterator_fn;
    void * iterator_arg;
    size_t *ret;
} __dns_local_iterate_req;
#endif
#if DNS_LOCAL_HOSTLIST
typedef struct {
    const char * hostname;
    ip_addr_t * addr;
    u8_t dns_addrtype;
    err_t *ret;
} __dns_local_lookup_req;
#endif
#if DNS_LOCAL_HOSTLIST && DNS_LOCAL_HOSTLIST_IS_DYNAMIC
typedef struct {
    const char * hostname;
    const ip_addr_t * addr;
    int *ret;
} __dns_local_removehost_req;
#endif
typedef struct {
    u8_t numdns;
    const ip_addr_t * dnsserver;
} __dns_setserver_req;
typedef struct {
    ;
} __dns_tmr_req;
