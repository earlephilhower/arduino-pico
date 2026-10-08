// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_IPV6 && LWIP_IPV6_MLD
typedef struct {
    struct pbuf * p;
    struct netif * inp;
} __mld6_input_req;
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
typedef struct {
    const ip6_addr_t * srcaddr;
    const ip6_addr_t * groupaddr;
    err_t *ret;
} __mld6_joingroup_req;
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
typedef struct {
    struct netif * netif;
    const ip6_addr_t * groupaddr;
    err_t *ret;
} __mld6_joingroup_netif_req;
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
typedef struct {
    const ip6_addr_t * srcaddr;
    const ip6_addr_t * groupaddr;
    err_t *ret;
} __mld6_leavegroup_req;
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
typedef struct {
    struct netif * netif;
    const ip6_addr_t * groupaddr;
    err_t *ret;
} __mld6_leavegroup_netif_req;
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
typedef struct {
    struct netif * ifp;
    const ip6_addr_t * addr;
    struct mld_group * *ret;
} __mld6_lookfor_group_req;
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
typedef struct {
    struct netif * netif;
} __mld6_report_groups_req;
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
typedef struct {
    struct netif * netif;
    err_t *ret;
} __mld6_stop_req;
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
typedef struct {
    ;
} __mld6_tmr_req;
#endif
