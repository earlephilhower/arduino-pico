// Machine generated (makewrapper.py).  DO NOT EDIT
typedef struct {
    struct netif * netif;
    s8_t slot;
    void * *ret;
} __mdns_get_service_txt_userdata_req;
typedef struct {
    struct netif * netif;
    const char * hostname;
    err_t *ret;
} __mdns_resp_add_netif_req;
typedef struct {
    struct netif * netif;
    const char * name;
    const char * service;
    enum mdns_sd_proto proto;
    u16_t port;
    service_get_txt_fn_t txt_fn;
    void * txt_userdata;
    s8_t *ret;
} __mdns_resp_add_service_req;
typedef struct {
    struct mdns_service * service;
    const char * txt;
    u8_t txt_len;
    err_t *ret;
} __mdns_resp_add_service_txtitem_req;
typedef struct {
    struct netif * netif;
} __mdns_resp_announce_req;
typedef struct {
    struct netif * netif;
    u8_t slot;
    err_t *ret;
} __mdns_resp_del_service_req;
typedef struct {
    ;
} __mdns_resp_init_req;
typedef struct {
    struct netif * netif;
    int *ret;
} __mdns_resp_netif_active_req;
typedef struct {
    mdns_name_result_cb_t cb;
} __mdns_resp_register_name_result_cb_req;
typedef struct {
    struct netif * netif;
    err_t *ret;
} __mdns_resp_remove_netif_req;
typedef struct {
    struct netif * netif;
    const char * hostname;
    err_t *ret;
} __mdns_resp_rename_netif_req;
typedef struct {
    struct netif * netif;
    u8_t slot;
    const char * name;
    err_t *ret;
} __mdns_resp_rename_service_req;
typedef struct {
    struct netif * netif;
} __mdns_resp_restart_req;
typedef struct {
    struct netif * netif;
    uint32_t delay;
} __mdns_resp_restart_delay_req;
typedef struct {
    const char * name;
    const char * service;
    enum mdns_sd_proto proto;
    struct netif * netif;
    search_result_fn_t result_fn;
    void * arg;
    u8_t * request_id;
    err_t *ret;
} __mdns_search_service_req;
typedef struct {
    u8_t request_id;
} __mdns_search_stop_req;
