// Machine generated (makewrapper.py).  DO NOT EDIT
extern u8_t __real_sntp_enabled();
extern u8_t __real_sntp_getkodreceived(u8_t idx);
extern u8_t __real_sntp_getoperatingmode();
extern u8_t __real_sntp_getreachability(u8_t idx);
extern const ip_addr_t * __real_sntp_getserver(u8_t idx);
extern const char * __real_sntp_getservername(u8_t idx);
extern void __real_sntp_init();
#if SNTP_GET_SERVERS_FROM_DHCP || SNTP_GET_SERVERS_FROM_DHCPV6
extern void __real_sntp_servermode_dhcp(int set_servers_from_dhcp);
#endif
extern void __real_sntp_setoperatingmode(u8_t operating_mode);
extern void __real_sntp_setserver(u8_t idx,const ip_addr_t * addr);
extern void __real_sntp_setservername(u8_t idx,const char * server);
extern void __real_sntp_stop();
