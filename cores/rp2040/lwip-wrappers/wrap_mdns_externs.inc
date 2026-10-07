// Machine generated (makewrapper.py).  DO NOT EDIT
extern void * __real_mdns_get_service_txt_userdata(struct netif * netif,s8_t slot);
extern err_t __real_mdns_resp_add_netif(struct netif * netif,const char * hostname);
extern s8_t __real_mdns_resp_add_service(struct netif * netif,const char * name,const char * service,enum mdns_sd_proto proto,u16_t port,service_get_txt_fn_t txt_fn,void * txt_userdata);
extern err_t __real_mdns_resp_add_service_txtitem(struct mdns_service * service,const char * txt,u8_t txt_len);
extern void __real_mdns_resp_announce(struct netif * netif);
extern err_t __real_mdns_resp_del_service(struct netif * netif,u8_t slot);
extern void __real_mdns_resp_init();
extern int __real_mdns_resp_netif_active(struct netif * netif);
extern void __real_mdns_resp_register_name_result_cb(mdns_name_result_cb_t cb);
extern err_t __real_mdns_resp_remove_netif(struct netif * netif);
extern err_t __real_mdns_resp_rename_netif(struct netif * netif,const char * hostname);
extern err_t __real_mdns_resp_rename_service(struct netif * netif,u8_t slot,const char * name);
extern void __real_mdns_resp_restart(struct netif * netif);
extern void __real_mdns_resp_restart_delay(struct netif * netif,uint32_t delay);
extern err_t __real_mdns_search_service(const char * name,const char * service,enum mdns_sd_proto proto,struct netif * netif,search_result_fn_t result_fn,void * arg,u8_t * request_id);
extern void __real_mdns_search_stop(u8_t request_id);
