// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_EVENT_API
extern err_t __real_lwip_tcp_event(void * arg,struct tcp_pcb * pcb,enum lwip_event,struct pbuf * p,u16_t size,err_t err);
#endif
extern void __real_tcp_abort(struct tcp_pcb * pcb);
extern void __real_tcp_accept(struct tcp_pcb * pcb,tcp_accept_fn accept);
extern void __real_tcp_arg(struct tcp_pcb * pcb,void * arg);
extern void __real_tcp_backlog_accepted(struct tcp_pcb * pcb);
extern void __real_tcp_backlog_delayed(struct tcp_pcb * pcb);
extern err_t __real_tcp_bind(struct tcp_pcb * pcb,const ip_addr_t * ipaddr,u16_t port);
extern void __real_tcp_bind_netif(struct tcp_pcb * pcb,const struct netif * netif);
extern err_t __real_tcp_close(struct tcp_pcb * pcb);
extern err_t __real_tcp_connect(struct tcp_pcb * pcb,const ip_addr_t * ipaddr,u16_t port,tcp_connected_fn connected);
extern void __real_tcp_err(struct tcp_pcb * pcb,tcp_err_fn err);
#if LWIP_TCP_PCB_NUM_EXT_ARGS
extern u8_t __real_tcp_ext_arg_alloc_id();
#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
extern void * __real_tcp_ext_arg_get(const struct tcp_pcb * pcb,u8_t id);
#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
extern void __real_tcp_ext_arg_set(struct tcp_pcb * pcb,u8_t id,void * arg);
#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
extern void __real_tcp_ext_arg_set_callbacks(struct tcp_pcb * pcb,u8_t id,const struct tcp_ext_arg_callbacks * const callbacks);
#endif
extern struct tcp_pcb * __real_tcp_listen_with_backlog(struct tcp_pcb * pcb,u8_t backlog);
extern struct tcp_pcb * __real_tcp_listen_with_backlog_and_err(struct tcp_pcb * pcb,u8_t backlog,err_t * err);
extern struct tcp_pcb * __real_tcp_new();
extern struct tcp_pcb * __real_tcp_new_ip_type(u8_t type);
extern err_t __real_tcp_output(struct tcp_pcb * pcb);
extern void __real_tcp_poll(struct tcp_pcb * pcb,tcp_poll_fn poll,u8_t interval);
extern void __real_tcp_recv(struct tcp_pcb * pcb,tcp_recv_fn recv);
extern void __real_tcp_recved(struct tcp_pcb * pcb,u16_t len);
extern void __real_tcp_sent(struct tcp_pcb * pcb,tcp_sent_fn sent);
extern void __real_tcp_setprio(struct tcp_pcb * pcb,u8_t prio);
extern err_t __real_tcp_shutdown(struct tcp_pcb * pcb,int shut_rx,int shut_tx);
extern err_t __real_tcp_tcp_get_tcp_addrinfo(struct tcp_pcb * pcb,int local,ip_addr_t * addr,u16_t * port);
extern err_t __real_tcp_write(struct tcp_pcb * pcb,const void * dataptr,u16_t len,u8_t apiflags);
