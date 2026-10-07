// Machine generated (makewrapper.py).  DO NOT EDIT
extern err_t __real_raw_bind(struct raw_pcb * pcb,const ip_addr_t * ipaddr);
extern void __real_raw_bind_netif(struct raw_pcb * pcb,const struct netif * netif);
extern err_t __real_raw_connect(struct raw_pcb * pcb,const ip_addr_t * ipaddr);
extern void __real_raw_disconnect(struct raw_pcb * pcb);
extern struct raw_pcb * __real_raw_new(u8_t proto);
extern struct raw_pcb * __real_raw_new_ip_type(u8_t type,u8_t proto);
extern void __real_raw_recv(struct raw_pcb * pcb,raw_recv_fn recv,void * recv_arg);
extern void __real_raw_remove(struct raw_pcb * pcb);
extern err_t __real_raw_send(struct raw_pcb * pcb,struct pbuf * p);
extern err_t __real_raw_sendto(struct raw_pcb * pcb,struct pbuf * p,const ip_addr_t * ipaddr);
extern err_t __real_raw_sendto_if_src(struct raw_pcb * pcb,struct pbuf * p,const ip_addr_t * dst_ip,struct netif * netif,const ip_addr_t * src_ip);
