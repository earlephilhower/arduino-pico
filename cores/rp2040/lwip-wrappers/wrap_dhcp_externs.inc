// Machine generated (makewrapper.py).  DO NOT EDIT
extern void __real_dhcp_cleanup(struct netif * netif);
extern void __real_dhcp_coarse_tmr();
extern void __real_dhcp_fine_tmr();
extern void __real_dhcp_inform(struct netif * netif);
extern void __real_dhcp_network_changed_link_up(struct netif * netif);
extern err_t __real_dhcp_release(struct netif * netif);
extern void __real_dhcp_release_and_stop(struct netif * netif);
extern err_t __real_dhcp_renew(struct netif * netif);
#if LWIP_DHCP && SNTP_GET_SERVERS_FROM_DHCP
extern void __real_dhcp_set_ntp_servers(u8_t num_ntp_servers,const ip4_addr_t * ntp_server_addrs);
#endif
extern void __real_dhcp_set_struct(struct netif * netif,struct dhcp * dhcp);
extern err_t __real_dhcp_start(struct netif * netif);
extern void __real_dhcp_stop(struct netif * netif);
extern u8_t __real_dhcp_supplied_address(const struct netif * netif);
