// Machine generated (makewrapper.py).  DO NOT EDIT
typedef struct {
    ;
} __igmp_init_req;
typedef struct {
    struct pbuf * p;
    struct netif * inp;
    const ip4_addr_t * dest;
} __igmp_input_req;
typedef struct {
    const ip4_addr_t * ifaddr;
    const ip4_addr_t * groupaddr;
    err_t *ret;
} __igmp_joingroup_req;
typedef struct {
    struct netif * netif;
    const ip4_addr_t * groupaddr;
    err_t *ret;
} __igmp_joingroup_netif_req;
typedef struct {
    const ip4_addr_t * ifaddr;
    const ip4_addr_t * groupaddr;
    err_t *ret;
} __igmp_leavegroup_req;
typedef struct {
    struct netif * netif;
    const ip4_addr_t * groupaddr;
    err_t *ret;
} __igmp_leavegroup_netif_req;
typedef struct {
    struct netif * ifp;
    const ip4_addr_t * addr;
    struct igmp_group * *ret;
} __igmp_lookfor_group_req;
typedef struct {
    struct netif * netif;
} __igmp_report_groups_req;
typedef struct {
    struct netif * netif;
    err_t *ret;
} __igmp_start_req;
typedef struct {
    struct netif * netif;
    err_t *ret;
} __igmp_stop_req;
typedef struct {
    ;
} __igmp_tmr_req;
