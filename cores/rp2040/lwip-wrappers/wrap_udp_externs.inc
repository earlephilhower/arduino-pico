// Machine generated (makewrapper.py).  DO NOT EDIT
extern err_t __real_udp_bind(struct udp_pcb * pcb,const ip_addr_t * ipaddr,u16_t port);
extern void __real_udp_bind_netif(struct udp_pcb * pcb,const struct netif * netif);
extern err_t __real_udp_connect(struct udp_pcb * pcb,const ip_addr_t * ipaddr,u16_t port);
#if UDP_DEBUG
extern void __real_udp_debug_print(struct udp_hdr * udphdr);
#endif
extern void __real_udp_disconnect(struct udp_pcb * pcb);
extern void __real_udp_init();
extern void __real_udp_input(struct pbuf * p,struct netif * inp);
extern void __real_udp_netif_ip_addr_changed(const ip_addr_t * old_addr,const ip_addr_t * new_addr);
extern struct udp_pcb * __real_udp_new();
extern struct udp_pcb * __real_udp_new_ip_type(u8_t type);
extern void __real_udp_recv(struct udp_pcb * pcb,udp_recv_fn recv,void * recv_arg);
extern void __real_udp_remove(struct udp_pcb * pcb);
extern err_t __real_udp_send(struct udp_pcb * pcb,struct pbuf * p);
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
extern err_t __real_udp_send_chksum(struct udp_pcb * pcb,struct pbuf * p,u8_t have_chksum,u16_t chksum);
#endif
extern err_t __real_udp_sendto(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port);
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
extern err_t __real_udp_sendto_chksum(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port,u8_t have_chksum,u16_t chksum);
#endif
extern err_t __real_udp_sendto_if(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port,struct netif * netif);
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
extern err_t __real_udp_sendto_if_chksum(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port,struct netif * netif,u8_t have_chksum,u16_t chksum);
#endif
extern err_t __real_udp_sendto_if_src(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port,struct netif * netif,const ip_addr_t * src_ip);
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
extern err_t __real_udp_sendto_if_src_chksum(struct udp_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,u16_t dst_port,struct netif * netif,u8_t have_chksum,u16_t chksum,const ip_addr_t * src_ip);
#endif
