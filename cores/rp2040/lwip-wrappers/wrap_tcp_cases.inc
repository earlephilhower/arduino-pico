// Machine generated (makewrapper.py).  DO NOT EDIT
#if LWIP_EVENT_API
case __lwip_tcp_event: {
    __lwip_tcp_event_req *r = (__lwip_tcp_event_req *)w.req;
    *(r->ret) = __real_lwip_tcp_event(r->arg, r->pcb, r->lwip_event, r->p, r->size, r->err);
    break;
}
#endif
case __tcp_abort: {
    __tcp_abort_req *r = (__tcp_abort_req *)w.req;
    __real_tcp_abort(r->pcb);
    break;
}
case __tcp_accept: {
    __tcp_accept_req *r = (__tcp_accept_req *)w.req;
    __real_tcp_accept(r->pcb, r->accept);
    break;
}
case __tcp_arg: {
    __tcp_arg_req *r = (__tcp_arg_req *)w.req;
    __real_tcp_arg(r->pcb, r->arg);
    break;
}
case __tcp_backlog_accepted: {
    __tcp_backlog_accepted_req *r = (__tcp_backlog_accepted_req *)w.req;
    __real_tcp_backlog_accepted(r->pcb);
    break;
}
case __tcp_backlog_delayed: {
    __tcp_backlog_delayed_req *r = (__tcp_backlog_delayed_req *)w.req;
    __real_tcp_backlog_delayed(r->pcb);
    break;
}
case __tcp_bind: {
    __tcp_bind_req *r = (__tcp_bind_req *)w.req;
    *(r->ret) = __real_tcp_bind(r->pcb, r->ipaddr, r->port);
    break;
}
case __tcp_bind_netif: {
    __tcp_bind_netif_req *r = (__tcp_bind_netif_req *)w.req;
    __real_tcp_bind_netif(r->pcb, r->netif);
    break;
}
case __tcp_close: {
    __tcp_close_req *r = (__tcp_close_req *)w.req;
    *(r->ret) = __real_tcp_close(r->pcb);
    break;
}
case __tcp_connect: {
    __tcp_connect_req *r = (__tcp_connect_req *)w.req;
    *(r->ret) = __real_tcp_connect(r->pcb, r->ipaddr, r->port, r->connected);
    break;
}
case __tcp_err: {
    __tcp_err_req *r = (__tcp_err_req *)w.req;
    __real_tcp_err(r->pcb, r->err);
    break;
}
#if LWIP_TCP_PCB_NUM_EXT_ARGS
case __tcp_ext_arg_alloc_id: {
    __tcp_ext_arg_alloc_id_req *r = (__tcp_ext_arg_alloc_id_req *)w.req;
    *(r->ret) = __real_tcp_ext_arg_alloc_id();
    break;
}
#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
case __tcp_ext_arg_get: {
    __tcp_ext_arg_get_req *r = (__tcp_ext_arg_get_req *)w.req;
    *(r->ret) = __real_tcp_ext_arg_get(r->pcb, r->id);
    break;
}
#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
case __tcp_ext_arg_set: {
    __tcp_ext_arg_set_req *r = (__tcp_ext_arg_set_req *)w.req;
    __real_tcp_ext_arg_set(r->pcb, r->id, r->arg);
    break;
}
#endif
#if LWIP_TCP_PCB_NUM_EXT_ARGS
case __tcp_ext_arg_set_callbacks: {
    __tcp_ext_arg_set_callbacks_req *r = (__tcp_ext_arg_set_callbacks_req *)w.req;
    __real_tcp_ext_arg_set_callbacks(r->pcb, r->id, r->callbacks);
    break;
}
#endif
case __tcp_listen_with_backlog: {
    __tcp_listen_with_backlog_req *r = (__tcp_listen_with_backlog_req *)w.req;
    *(r->ret) = __real_tcp_listen_with_backlog(r->pcb, r->backlog);
    break;
}
case __tcp_listen_with_backlog_and_err: {
    __tcp_listen_with_backlog_and_err_req *r = (__tcp_listen_with_backlog_and_err_req *)w.req;
    *(r->ret) = __real_tcp_listen_with_backlog_and_err(r->pcb, r->backlog, r->err);
    break;
}
case __tcp_new: {
    __tcp_new_req *r = (__tcp_new_req *)w.req;
    *(r->ret) = __real_tcp_new();
    break;
}
case __tcp_new_ip_type: {
    __tcp_new_ip_type_req *r = (__tcp_new_ip_type_req *)w.req;
    *(r->ret) = __real_tcp_new_ip_type(r->type);
    break;
}
case __tcp_output: {
    __tcp_output_req *r = (__tcp_output_req *)w.req;
    *(r->ret) = __real_tcp_output(r->pcb);
    break;
}
case __tcp_poll: {
    __tcp_poll_req *r = (__tcp_poll_req *)w.req;
    __real_tcp_poll(r->pcb, r->poll, r->interval);
    break;
}
case __tcp_recv: {
    __tcp_recv_req *r = (__tcp_recv_req *)w.req;
    __real_tcp_recv(r->pcb, r->recv);
    break;
}
case __tcp_recved: {
    __tcp_recved_req *r = (__tcp_recved_req *)w.req;
    __real_tcp_recved(r->pcb, r->len);
    break;
}
case __tcp_sent: {
    __tcp_sent_req *r = (__tcp_sent_req *)w.req;
    __real_tcp_sent(r->pcb, r->sent);
    break;
}
case __tcp_setprio: {
    __tcp_setprio_req *r = (__tcp_setprio_req *)w.req;
    __real_tcp_setprio(r->pcb, r->prio);
    break;
}
case __tcp_shutdown: {
    __tcp_shutdown_req *r = (__tcp_shutdown_req *)w.req;
    *(r->ret) = __real_tcp_shutdown(r->pcb, r->shut_rx, r->shut_tx);
    break;
}
case __tcp_tcp_get_tcp_addrinfo: {
    __tcp_tcp_get_tcp_addrinfo_req *r = (__tcp_tcp_get_tcp_addrinfo_req *)w.req;
    *(r->ret) = __real_tcp_tcp_get_tcp_addrinfo(r->pcb, r->local, r->addr, r->port);
    break;
}
case __tcp_write: {
    __tcp_write_req *r = (__tcp_write_req *)w.req;
    *(r->ret) = __real_tcp_write(r->pcb, r->dataptr, r->len, r->apiflags);
    break;
}
