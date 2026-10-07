// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_TESTMODE
void __wrap_lwip_cyclic_timer(void * arg) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __lwip_cyclic_timer_req req = { arg };
        __lwip(__lwip_cyclic_timer, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_lwip_cyclic_timer(arg);
}

#endif
void __wrap_sys_check_timeouts() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sys_check_timeouts_req req = {  };
        __lwip(__sys_check_timeouts, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sys_check_timeouts();
}

void __wrap_sys_restart_timeouts() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sys_restart_timeouts_req req = {  };
        __lwip(__sys_restart_timeouts, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sys_restart_timeouts();
}

#if !LWIP_DEBUG_TIMERNAMES
void __wrap_sys_timeout(u32_t msecs,sys_timeout_handler handler,void * arg) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sys_timeout_req req = { msecs, handler, arg };
        __lwip(__sys_timeout, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sys_timeout(msecs, handler, arg);
}

#endif
#if LWIP_DEBUG_TIMERNAMES
void __wrap_sys_timeout_debug(u32_t msecs,sys_timeout_handler handler,void * arg,const char * handler_name) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sys_timeout_debug_req req = { msecs, handler, arg, handler_name };
        __lwip(__sys_timeout_debug, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sys_timeout_debug(msecs, handler, arg, handler_name);
}

#endif
#if LWIP_TESTMODE
struct sys_timeo ** __wrap_sys_timeouts_get_next_timeout() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct sys_timeo ** ret;
        __sys_timeouts_get_next_timeout_req req = { &ret };
        __lwip(__sys_timeouts_get_next_timeout, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_sys_timeouts_get_next_timeout();
}

#endif
void __wrap_sys_timeouts_init() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sys_timeouts_init_req req = {  };
        __lwip(__sys_timeouts_init, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sys_timeouts_init();
}

u32_t __wrap_sys_timeouts_sleeptime() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u32_t ret;
        __sys_timeouts_sleeptime_req req = { &ret };
        __lwip(__sys_timeouts_sleeptime, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_sys_timeouts_sleeptime();
}

void __wrap_sys_untimeout(sys_timeout_handler handler,void * arg) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __sys_untimeout_req req = { handler, arg };
        __lwip(__sys_untimeout, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_sys_untimeout(handler, arg);
}

