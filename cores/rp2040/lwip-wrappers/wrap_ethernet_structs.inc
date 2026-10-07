// Machine generated (makewrapper.py).  DO NOT EDIT
typedef struct {
    struct pbuf * p;
    struct netif * netif;
    err_t *ret;
} __ethernet_input_req;
typedef struct {
    struct netif * netif;
    struct pbuf * p;
    const struct eth_addr * src;
    const struct eth_addr * dst;
    u16_t eth_type;
    err_t *ret;
} __ethernet_output_req;
