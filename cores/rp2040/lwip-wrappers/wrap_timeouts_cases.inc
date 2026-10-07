// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_TESTMODE
case __lwip_cyclic_timer: {
    __lwip_cyclic_timer_req *r = (__lwip_cyclic_timer_req *)w.req;
    __real_lwip_cyclic_timer(r->arg);
    break;
}
#endif
case __sys_check_timeouts: {
    __real_sys_check_timeouts();
    break;
}
case __sys_restart_timeouts: {
    __real_sys_restart_timeouts();
    break;
}
#if !LWIP_DEBUG_TIMERNAMES
case __sys_timeout: {
    __sys_timeout_req *r = (__sys_timeout_req *)w.req;
    __real_sys_timeout(r->msecs, r->handler, r->arg);
    break;
}
#endif
#if LWIP_DEBUG_TIMERNAMES
case __sys_timeout_debug: {
    __sys_timeout_debug_req *r = (__sys_timeout_debug_req *)w.req;
    __real_sys_timeout_debug(r->msecs, r->handler, r->arg, r->handler_name);
    break;
}
#endif
#if LWIP_TESTMODE
case __sys_timeouts_get_next_timeout: {
    __sys_timeouts_get_next_timeout_req *r = (__sys_timeouts_get_next_timeout_req *)w.req;
    *(r->ret) = __real_sys_timeouts_get_next_timeout();
    break;
}
#endif
case __sys_timeouts_init: {
    __real_sys_timeouts_init();
    break;
}
case __sys_timeouts_sleeptime: {
    __sys_timeouts_sleeptime_req *r = (__sys_timeouts_sleeptime_req *)w.req;
    *(r->ret) = __real_sys_timeouts_sleeptime();
    break;
}
case __sys_untimeout: {
    __sys_untimeout_req *r = (__sys_untimeout_req *)w.req;
    __real_sys_untimeout(r->handler, r->arg);
    break;
}
