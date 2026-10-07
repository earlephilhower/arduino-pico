// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_TESTMODE
typedef struct {
    void * arg;
} __lwip_cyclic_timer_req;
#endif
typedef struct {
    ;
} __sys_check_timeouts_req;
typedef struct {
    ;
} __sys_restart_timeouts_req;
#if !LWIP_DEBUG_TIMERNAMES
typedef struct {
    u32_t msecs;
    sys_timeout_handler handler;
    void * arg;
} __sys_timeout_req;
#endif
#if LWIP_DEBUG_TIMERNAMES
typedef struct {
    u32_t msecs;
    sys_timeout_handler handler;
    void * arg;
    const char * handler_name;
} __sys_timeout_debug_req;
#endif
#if LWIP_TESTMODE
typedef struct {
    ;
    struct sys_timeo ** *ret;
} __sys_timeouts_get_next_timeout_req;
#endif
typedef struct {
    ;
} __sys_timeouts_init_req;
typedef struct {
    ;
    u32_t *ret;
} __sys_timeouts_sleeptime_req;
typedef struct {
    sys_timeout_handler handler;
    void * arg;
} __sys_untimeout_req;
