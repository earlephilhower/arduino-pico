// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_TESTMODE
extern void __real_lwip_cyclic_timer(void * arg);
#endif
extern void __real_sys_check_timeouts();
extern void __real_sys_restart_timeouts();
#if !LWIP_DEBUG_TIMERNAMES
extern void __real_sys_timeout(u32_t msecs,sys_timeout_handler handler,void * arg);
#endif
#if LWIP_DEBUG_TIMERNAMES
extern void __real_sys_timeout_debug(u32_t msecs,sys_timeout_handler handler,void * arg,const char * handler_name);
#endif
#if LWIP_TESTMODE
extern struct sys_timeo ** __real_sys_timeouts_get_next_timeout();
#endif
extern void __real_sys_timeouts_init();
extern u32_t __real_sys_timeouts_sleeptime();
extern void __real_sys_untimeout(sys_timeout_handler handler,void * arg);
