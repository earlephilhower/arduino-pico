// Machine generated (makewrapper.py).  DO NOT EDIT
extern void __real_igmp_init();
extern void __real_igmp_input(struct pbuf * p,struct netif * inp,const ip4_addr_t * dest);
extern err_t __real_igmp_joingroup(const ip4_addr_t * ifaddr,const ip4_addr_t * groupaddr);
extern err_t __real_igmp_joingroup_netif(struct netif * netif,const ip4_addr_t * groupaddr);
extern err_t __real_igmp_leavegroup(const ip4_addr_t * ifaddr,const ip4_addr_t * groupaddr);
extern err_t __real_igmp_leavegroup_netif(struct netif * netif,const ip4_addr_t * groupaddr);
extern struct igmp_group * __real_igmp_lookfor_group(struct netif * ifp,const ip4_addr_t * addr);
extern void __real_igmp_report_groups(struct netif * netif);
extern err_t __real_igmp_start(struct netif * netif);
extern err_t __real_igmp_stop(struct netif * netif);
extern void __real_igmp_tmr();
