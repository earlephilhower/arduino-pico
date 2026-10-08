// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_IPV6 && LWIP_IPV6_MLD
extern void __real_mld6_input(struct pbuf * p,struct netif * inp);
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
extern err_t __real_mld6_joingroup(const ip6_addr_t * srcaddr,const ip6_addr_t * groupaddr);
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
extern err_t __real_mld6_joingroup_netif(struct netif * netif,const ip6_addr_t * groupaddr);
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
extern err_t __real_mld6_leavegroup(const ip6_addr_t * srcaddr,const ip6_addr_t * groupaddr);
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
extern err_t __real_mld6_leavegroup_netif(struct netif * netif,const ip6_addr_t * groupaddr);
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
extern struct mld_group * __real_mld6_lookfor_group(struct netif * ifp,const ip6_addr_t * addr);
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
extern void __real_mld6_report_groups(struct netif * netif);
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
extern err_t __real_mld6_stop(struct netif * netif);
#endif
#if LWIP_IPV6 && LWIP_IPV6_MLD
extern void __real_mld6_tmr();
#endif
