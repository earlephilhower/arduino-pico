// Machine generated (makewrapper.py).  DO NOT EDIT
extern struct netif * __real_netif_add(struct netif * netif,const ip4_addr_t * ipaddr,const ip4_addr_t * netmask,const ip4_addr_t * gw,void * state,netif_init_fn init,netif_input_fn input);
#if LWIP_NETIF_EXT_STATUS_CALLBACK
extern void __real_netif_add_ext_callback(netif_ext_callback_t * callback,netif_ext_callback_fn fn);
#endif
#if LWIP_IPV6
extern err_t __real_netif_add_ip6_address(struct netif * netif,const ip6_addr_t * ip6addr,s8_t * chosen_idx);
#endif
extern struct netif * __real_netif_add_noaddr(struct netif * netif,void * state,netif_init_fn init,netif_input_fn input);
extern u8_t __real_netif_alloc_client_data_id();
#if LWIP_IPV6
extern void __real_netif_create_ip6_linklocal_address(struct netif * netif,u8_t from_mac_48bit);
#endif
extern struct netif * __real_netif_find(const char * name);
extern struct netif * __real_netif_get_by_index(u8_t idx);
#if LWIP_IPV6
extern s8_t __real_netif_get_ip6_addr_match(struct netif * netif,const ip6_addr_t * ip6addr);
#endif
#if LWIP_TESTMODE && LWIP_HAVE_LOOPIF
extern struct netif * __real_netif_get_loopif();
#endif
extern char * __real_netif_index_to_name(u8_t idx,char * name);
extern void __real_netif_init();
extern err_t __real_netif_input(struct pbuf * p,struct netif * inp);
#if LWIP_NETIF_EXT_STATUS_CALLBACK
extern void __real_netif_invoke_ext_callback(struct netif * netif,netif_nsc_reason_t reason,const netif_ext_callback_args_t * args);
#endif
#if LWIP_IPV6
extern void __real_netif_ip6_addr_set(struct netif * netif,s8_t addr_idx,const ip6_addr_t * addr6);
#endif
#if LWIP_IPV6
extern void __real_netif_ip6_addr_set_parts(struct netif * netif,s8_t addr_idx,u32_t i0,u32_t i1,u32_t i2,u32_t i3);
#endif
#if LWIP_IPV6
extern void __real_netif_ip6_addr_set_state(struct netif * netif,s8_t addr_idx,u8_t state);
#endif
#if ENABLE_LOOPBACK
extern err_t __real_netif_loop_output(struct netif * netif,struct pbuf * p);
#endif
extern u8_t __real_netif_name_to_index(const char * name);
#if ENABLE_LOOPBACK
extern void __real_netif_poll(struct netif * netif);
#endif
#if ENABLE_LOOPBACK && !LWIP_NETIF_LOOPBACK_MULTITHREADING
extern void __real_netif_poll_all();
#endif
extern void __real_netif_remove(struct netif * netif);
#if LWIP_NETIF_EXT_STATUS_CALLBACK
extern void __real_netif_remove_ext_callback(netif_ext_callback_t * callback);
#endif
extern void __real_netif_set_addr(struct netif * netif,const ip4_addr_t * ipaddr,const ip4_addr_t * netmask,const ip4_addr_t * gw);
extern void __real_netif_set_default(struct netif * netif);
extern void __real_netif_set_down(struct netif * netif);
extern void __real_netif_set_gw(struct netif * netif,const ip4_addr_t * gw);
extern void __real_netif_set_ipaddr(struct netif * netif,const ip4_addr_t * ipaddr);
extern void __real_netif_set_link_callback(struct netif * netif,netif_status_callback_fn link_callback);
extern void __real_netif_set_link_down(struct netif * netif);
extern void __real_netif_set_link_up(struct netif * netif);
extern void __real_netif_set_netmask(struct netif * netif,const ip4_addr_t * netmask);
#if LWIP_NETIF_REMOVE_CALLBACK
extern void __real_netif_set_remove_callback(struct netif * netif,netif_status_callback_fn remove_callback);
#endif
extern void __real_netif_set_status_callback(struct netif * netif,netif_status_callback_fn status_callback);
extern void __real_netif_set_up(struct netif * netif);
