// Machine generated (makewrapper.py).  DO NOT EDIT
typedef struct {
    struct netif * netif;
    const ip4_addr_t * ipaddr;
    const ip4_addr_t * netmask;
    const ip4_addr_t * gw;
    void * state;
    netif_init_fn init;
    netif_input_fn input;
    struct netif * *ret;
} __netif_add_req;
#if LWIP_NETIF_EXT_STATUS_CALLBACK
typedef struct {
    netif_ext_callback_t * callback;
    netif_ext_callback_fn fn;
} __netif_add_ext_callback_req;
#endif
#if LWIP_IPV6
typedef struct {
    struct netif * netif;
    const ip6_addr_t * ip6addr;
    s8_t * chosen_idx;
    err_t *ret;
} __netif_add_ip6_address_req;
#endif
typedef struct {
    struct netif * netif;
    void * state;
    netif_init_fn init;
    netif_input_fn input;
    struct netif * *ret;
} __netif_add_noaddr_req;
typedef struct {
    ;
    u8_t *ret;
} __netif_alloc_client_data_id_req;
#if LWIP_IPV6
typedef struct {
    struct netif * netif;
    u8_t from_mac_48bit;
} __netif_create_ip6_linklocal_address_req;
#endif
typedef struct {
    const char * name;
    struct netif * *ret;
} __netif_find_req;
typedef struct {
    u8_t idx;
    struct netif * *ret;
} __netif_get_by_index_req;
#if LWIP_IPV6
typedef struct {
    struct netif * netif;
    const ip6_addr_t * ip6addr;
    s8_t *ret;
} __netif_get_ip6_addr_match_req;
#endif
#if LWIP_TESTMODE && LWIP_HAVE_LOOPIF
typedef struct {
    ;
    struct netif * *ret;
} __netif_get_loopif_req;
#endif
typedef struct {
    u8_t idx;
    char * name;
    char * *ret;
} __netif_index_to_name_req;
typedef struct {
    ;
} __netif_init_req;
typedef struct {
    struct pbuf * p;
    struct netif * inp;
    err_t *ret;
} __netif_input_req;
#if LWIP_NETIF_EXT_STATUS_CALLBACK
typedef struct {
    struct netif * netif;
    netif_nsc_reason_t reason;
    const netif_ext_callback_args_t * args;
} __netif_invoke_ext_callback_req;
#endif
#if LWIP_IPV6
typedef struct {
    struct netif * netif;
    s8_t addr_idx;
    const ip6_addr_t * addr6;
} __netif_ip6_addr_set_req;
#endif
#if LWIP_IPV6
typedef struct {
    struct netif * netif;
    s8_t addr_idx;
    u32_t i0;
    u32_t i1;
    u32_t i2;
    u32_t i3;
} __netif_ip6_addr_set_parts_req;
#endif
#if LWIP_IPV6
typedef struct {
    struct netif * netif;
    s8_t addr_idx;
    u8_t state;
} __netif_ip6_addr_set_state_req;
#endif
#if ENABLE_LOOPBACK
typedef struct {
    struct netif * netif;
    struct pbuf * p;
    err_t *ret;
} __netif_loop_output_req;
#endif
typedef struct {
    const char * name;
    u8_t *ret;
} __netif_name_to_index_req;
#if ENABLE_LOOPBACK
typedef struct {
    struct netif * netif;
} __netif_poll_req;
#endif
#if ENABLE_LOOPBACK && !LWIP_NETIF_LOOPBACK_MULTITHREADING
typedef struct {
    ;
} __netif_poll_all_req;
#endif
typedef struct {
    struct netif * netif;
} __netif_remove_req;
#if LWIP_NETIF_EXT_STATUS_CALLBACK
typedef struct {
    netif_ext_callback_t * callback;
} __netif_remove_ext_callback_req;
#endif
typedef struct {
    struct netif * netif;
    const ip4_addr_t * ipaddr;
    const ip4_addr_t * netmask;
    const ip4_addr_t * gw;
} __netif_set_addr_req;
typedef struct {
    struct netif * netif;
} __netif_set_default_req;
typedef struct {
    struct netif * netif;
} __netif_set_down_req;
typedef struct {
    struct netif * netif;
    const ip4_addr_t * gw;
} __netif_set_gw_req;
typedef struct {
    struct netif * netif;
    const ip4_addr_t * ipaddr;
} __netif_set_ipaddr_req;
typedef struct {
    struct netif * netif;
    netif_status_callback_fn link_callback;
} __netif_set_link_callback_req;
typedef struct {
    struct netif * netif;
} __netif_set_link_down_req;
typedef struct {
    struct netif * netif;
} __netif_set_link_up_req;
typedef struct {
    struct netif * netif;
    const ip4_addr_t * netmask;
} __netif_set_netmask_req;
#if LWIP_NETIF_REMOVE_CALLBACK
typedef struct {
    struct netif * netif;
    netif_status_callback_fn remove_callback;
} __netif_set_remove_callback_req;
#endif
typedef struct {
    struct netif * netif;
    netif_status_callback_fn status_callback;
} __netif_set_status_callback_req;
typedef struct {
    struct netif * netif;
} __netif_set_up_req;
