// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_EVENT_API
typedef struct {
    void * arg;
    struct tcp_pcb * pcb;
    enum lwip_event;
    struct pbuf * p;
    u16_t size;
    err_t err;
    err_t *ret;
} __lwip_tcp_event_req;
#endif
typedef struct {
    struct tcp_pcb * pcb;
} __tcp_abort_req;
typedef struct {
    struct tcp_pcb * pcb;
    tcp_accept_fn accept;
} __tcp_accept_req;
typedef struct {
    struct tcp_pcb * pcb;
    void * arg;
} __tcp_arg_req;
typedef struct {
    struct tcp_pcb * pcb;
} __tcp_backlog_accepted_req;
typedef struct {
    struct tcp_pcb * pcb;
} __tcp_backlog_delayed_req;
typedef struct {
    struct tcp_pcb * pcb;
    const ip_addr_t * ipaddr;
    u16_t port;
    err_t *ret;
} __tcp_bind_req;
typedef struct {
    struct tcp_pcb * pcb;
    const struct netif * netif;
} __tcp_bind_netif_req;
typedef struct {
    struct tcp_pcb * pcb;
    err_t *ret;
} __tcp_close_req;
typedef struct {
    struct tcp_pcb * pcb;
    const ip_addr_t * ipaddr;
    u16_t port;
    tcp_connected_fn connected;
    err_t *ret;
} __tcp_connect_req;
typedef struct {
    struct tcp_pcb * pcb;
    tcp_err_fn err;
} __tcp_err_req;
#if LWIP_TCP_PCB_NUM_EXT_ARGS
typedef struct {
    ;
    u8_t *ret;
} __tcp_ext_arg_alloc_id_req;
#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
typedef struct {
    const struct tcp_pcb * pcb;
    u8_t id;
    void * *ret;
} __tcp_ext_arg_get_req;
#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
typedef struct {
    struct tcp_pcb * pcb;
    u8_t id;
    void * arg;
} __tcp_ext_arg_set_req;
#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
typedef struct {
    struct tcp_pcb * pcb;
    u8_t id;
    const struct tcp_ext_arg_callbacks * const callbacks;
} __tcp_ext_arg_set_callbacks_req;
#endif
typedef struct {
    struct tcp_pcb * pcb;
    u8_t backlog;
    struct tcp_pcb * *ret;
} __tcp_listen_with_backlog_req;
typedef struct {
    struct tcp_pcb * pcb;
    u8_t backlog;
    err_t * err;
    struct tcp_pcb * *ret;
} __tcp_listen_with_backlog_and_err_req;
typedef struct {
    ;
    struct tcp_pcb * *ret;
} __tcp_new_req;
typedef struct {
    u8_t type;
    struct tcp_pcb * *ret;
} __tcp_new_ip_type_req;
typedef struct {
    struct tcp_pcb * pcb;
    err_t *ret;
} __tcp_output_req;
typedef struct {
    struct tcp_pcb * pcb;
    tcp_poll_fn poll;
    u8_t interval;
} __tcp_poll_req;
typedef struct {
    struct tcp_pcb * pcb;
    tcp_recv_fn recv;
} __tcp_recv_req;
typedef struct {
    struct tcp_pcb * pcb;
    u16_t len;
} __tcp_recved_req;
typedef struct {
    struct tcp_pcb * pcb;
    tcp_sent_fn sent;
} __tcp_sent_req;
typedef struct {
    struct tcp_pcb * pcb;
    u8_t prio;
} __tcp_setprio_req;
typedef struct {
    struct tcp_pcb * pcb;
    int shut_rx;
    int shut_tx;
    err_t *ret;
} __tcp_shutdown_req;
typedef struct {
    struct tcp_pcb * pcb;
    int local;
    ip_addr_t * addr;
    u16_t * port;
    err_t *ret;
} __tcp_tcp_get_tcp_addrinfo_req;
typedef struct {
    struct tcp_pcb * pcb;
    const void * dataptr;
    u16_t len;
    u8_t apiflags;
    err_t *ret;
} __tcp_write_req;
