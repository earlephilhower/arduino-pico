// Machine generated (makewrapper.py).  DO NOT EDIT
typedef struct {
    struct raw_pcb * pcb;
    const ip_addr_t * ipaddr;
    err_t *ret;
} __raw_bind_req;
typedef struct {
    struct raw_pcb * pcb;
    const struct netif * netif;
} __raw_bind_netif_req;
typedef struct {
    struct raw_pcb * pcb;
    const ip_addr_t * ipaddr;
    err_t *ret;
} __raw_connect_req;
typedef struct {
    struct raw_pcb * pcb;
} __raw_disconnect_req;
typedef struct {
    u8_t proto;
    struct raw_pcb * *ret;
} __raw_new_req;
typedef struct {
    u8_t type;
    u8_t proto;
    struct raw_pcb * *ret;
} __raw_new_ip_type_req;
typedef struct {
    struct raw_pcb * pcb;
    raw_recv_fn recv;
    void * recv_arg;
} __raw_recv_req;
typedef struct {
    struct raw_pcb * pcb;
} __raw_remove_req;
typedef struct {
    struct raw_pcb * pcb;
    struct pbuf * p;
    err_t *ret;
} __raw_send_req;
typedef struct {
    struct raw_pcb * pcb;
    struct pbuf * p;
    const ip_addr_t * ipaddr;
    err_t *ret;
} __raw_sendto_req;
typedef struct {
    struct raw_pcb * pcb;
    struct pbuf * p;
    const ip_addr_t * dst_ip;
    struct netif * netif;
    const ip_addr_t * src_ip;
    err_t *ret;
} __raw_sendto_if_src_req;
