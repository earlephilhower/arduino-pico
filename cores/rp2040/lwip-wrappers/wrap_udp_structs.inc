// Machine generated (makewrapper.py).  DO NOT EDIT
typedef struct {
    struct udp_pcb * pcb;
    const ip_addr_t * ipaddr;
    u16_t port;
    err_t *ret;
} __udp_bind_req;
typedef struct {
    struct udp_pcb * pcb;
    const struct netif * netif;
} __udp_bind_netif_req;
typedef struct {
    struct udp_pcb * pcb;
    const ip_addr_t * ipaddr;
    u16_t port;
    err_t *ret;
} __udp_connect_req;
#if UDP_DEBUG
typedef struct {
    struct udp_hdr * udphdr;
} __udp_debug_print_req;
#endif
typedef struct {
    struct udp_pcb * pcb;
} __udp_disconnect_req;
typedef struct {
    ;
} __udp_init_req;
typedef struct {
    struct pbuf * p;
    struct netif * inp;
} __udp_input_req;
typedef struct {
    const ip_addr_t * old_addr;
    const ip_addr_t * new_addr;
} __udp_netif_ip_addr_changed_req;
typedef struct {
    ;
    struct udp_pcb * *ret;
} __udp_new_req;
typedef struct {
    u8_t type;
    struct udp_pcb * *ret;
} __udp_new_ip_type_req;
typedef struct {
    struct udp_pcb * pcb;
    udp_recv_fn recv;
    void * recv_arg;
} __udp_recv_req;
typedef struct {
    struct udp_pcb * pcb;
} __udp_remove_req;
typedef struct {
    struct udp_pcb * pcb;
    struct pbuf * p;
    err_t *ret;
} __udp_send_req;
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
typedef struct {
    struct udp_pcb * pcb;
    struct pbuf * p;
    u8_t have_chksum;
    u16_t chksum;
    err_t *ret;
} __udp_send_chksum_req;
#endif
typedef struct {
    struct udp_pcb * pcb;
    struct pbuf * p;
    const ip_addr_t * dst_ip;
    u16_t dst_port;
    err_t *ret;
} __udp_sendto_req;
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
typedef struct {
    struct udp_pcb * pcb;
    struct pbuf * p;
    const ip_addr_t * dst_ip;
    u16_t dst_port;
    u8_t have_chksum;
    u16_t chksum;
    err_t *ret;
} __udp_sendto_chksum_req;
#endif
typedef struct {
    struct udp_pcb * pcb;
    struct pbuf * p;
    const ip_addr_t * dst_ip;
    u16_t dst_port;
    struct netif * netif;
    err_t *ret;
} __udp_sendto_if_req;
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
typedef struct {
    struct udp_pcb * pcb;
    struct pbuf * p;
    const ip_addr_t * dst_ip;
    u16_t dst_port;
    struct netif * netif;
    u8_t have_chksum;
    u16_t chksum;
    err_t *ret;
} __udp_sendto_if_chksum_req;
#endif
typedef struct {
    struct udp_pcb * pcb;
    struct pbuf * p;
    const ip_addr_t * dst_ip;
    u16_t dst_port;
    struct netif * netif;
    const ip_addr_t * src_ip;
    err_t *ret;
} __udp_sendto_if_src_req;
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
typedef struct {
    struct udp_pcb * pcb;
    struct pbuf * p;
    const ip_addr_t * dst_ip;
    u16_t dst_port;
    struct netif * netif;
    u8_t have_chksum;
    u16_t chksum;
    const ip_addr_t * src_ip;
    err_t *ret;
} __udp_sendto_if_src_chksum_req;
#endif
