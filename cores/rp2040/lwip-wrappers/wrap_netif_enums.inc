// Machine generated (makewrapper.py).  DO NOT EDIT
__netif_add,
#if LWIP_NETIF_EXT_STATUS_CALLBACK
__netif_add_ext_callback,
#endif
#if LWIP_IPV6
__netif_add_ip6_address,
#endif
__netif_add_noaddr,
__netif_alloc_client_data_id,
#if LWIP_IPV6
__netif_create_ip6_linklocal_address,
#endif
__netif_find,
__netif_get_by_index,
#if LWIP_IPV6
__netif_get_ip6_addr_match,
#endif
#if LWIP_TESTMODE && LWIP_HAVE_LOOPIF
__netif_get_loopif,
#endif
__netif_index_to_name,
__netif_init,
__netif_input,
#if LWIP_NETIF_EXT_STATUS_CALLBACK
__netif_invoke_ext_callback,
#endif
#if LWIP_IPV6
__netif_ip6_addr_set,
#endif
#if LWIP_IPV6
__netif_ip6_addr_set_parts,
#endif
#if LWIP_IPV6
__netif_ip6_addr_set_state,
#endif
#if ENABLE_LOOPBACK
__netif_loop_output,
#endif
__netif_name_to_index,
#if ENABLE_LOOPBACK
__netif_poll,
#endif
#if ENABLE_LOOPBACK && !LWIP_NETIF_LOOPBACK_MULTITHREADING
__netif_poll_all,
#endif
__netif_remove,
#if LWIP_NETIF_EXT_STATUS_CALLBACK
__netif_remove_ext_callback,
#endif
__netif_set_addr,
__netif_set_default,
__netif_set_down,
__netif_set_gw,
__netif_set_ipaddr,
__netif_set_link_callback,
__netif_set_link_down,
__netif_set_link_up,
__netif_set_netmask,
#if LWIP_NETIF_REMOVE_CALLBACK
__netif_set_remove_callback,
#endif
__netif_set_status_callback,
__netif_set_up,
