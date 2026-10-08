// Machine generated (makewrapper.py).  DO NOT EDIT
typedef struct {
    ;
    u8_t *ret;
} __sntp_enabled_req;
typedef struct {
    u8_t idx;
    u8_t *ret;
} __sntp_getkodreceived_req;
typedef struct {
    ;
    u8_t *ret;
} __sntp_getoperatingmode_req;
typedef struct {
    u8_t idx;
    u8_t *ret;
} __sntp_getreachability_req;
typedef struct {
    u8_t idx;
    const ip_addr_t * *ret;
} __sntp_getserver_req;
typedef struct {
    u8_t idx;
    const char * *ret;
} __sntp_getservername_req;
typedef struct {
    ;
} __sntp_init_req;
#if SNTP_GET_SERVERS_FROM_DHCP || SNTP_GET_SERVERS_FROM_DHCPV6
typedef struct {
    int set_servers_from_dhcp;
} __sntp_servermode_dhcp_req;
#endif
typedef struct {
    u8_t operating_mode;
} __sntp_setoperatingmode_req;
typedef struct {
    u8_t idx;
    const ip_addr_t * addr;
} __sntp_setserver_req;
typedef struct {
    u8_t idx;
    const char * server;
} __sntp_setservername_req;
typedef struct {
    ;
} __sntp_stop_req;
