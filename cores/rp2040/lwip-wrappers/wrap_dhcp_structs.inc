// Machine generated (makewrapper.py).  DO NOT EDIT
typedef struct {
    struct netif * netif;
} __dhcp_cleanup_req;
typedef struct {
    ;
} __dhcp_coarse_tmr_req;
typedef struct {
    ;
} __dhcp_fine_tmr_req;
typedef struct {
    struct netif * netif;
} __dhcp_inform_req;
typedef struct {
    struct netif * netif;
} __dhcp_network_changed_link_up_req;
typedef struct {
    struct netif * netif;
    err_t *ret;
} __dhcp_release_req;
typedef struct {
    struct netif * netif;
} __dhcp_release_and_stop_req;
typedef struct {
    struct netif * netif;
    err_t *ret;
} __dhcp_renew_req;
#if LWIP_DHCP && SNTP_GET_SERVERS_FROM_DHCP
typedef struct {
    u8_t num_ntp_servers;
    const ip4_addr_t * ntp_server_addrs;
} __dhcp_set_ntp_servers_req;
#endif
typedef struct {
    struct netif * netif;
    struct dhcp * dhcp;
} __dhcp_set_struct_req;
typedef struct {
    struct netif * netif;
    err_t *ret;
} __dhcp_start_req;
typedef struct {
    struct netif * netif;
} __dhcp_stop_req;
typedef struct {
    const struct netif * netif;
    u8_t *ret;
} __dhcp_supplied_address_req;
