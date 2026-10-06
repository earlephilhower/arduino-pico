// Machine generated (makewrapper.py).  DO NOT EDIT
case __raw_bind: {
    __raw_bind_req *r = (__raw_bind_req *)w.req;
    *(r->ret) = __real_raw_bind(r->pcb, r->ipaddr);
    break;
}
case __raw_bind_netif: {
    __raw_bind_netif_req *r = (__raw_bind_netif_req *)w.req;
    __real_raw_bind_netif(r->pcb, r->netif);
    break;
}
case __raw_connect: {
    __raw_connect_req *r = (__raw_connect_req *)w.req;
    *(r->ret) = __real_raw_connect(r->pcb, r->ipaddr);
    break;
}
case __raw_disconnect: {
    __raw_disconnect_req *r = (__raw_disconnect_req *)w.req;
    __real_raw_disconnect(r->pcb);
    break;
}
case __raw_new: {
    __raw_new_req *r = (__raw_new_req *)w.req;
    *(r->ret) = __real_raw_new(r->proto);
    break;
}
case __raw_new_ip_type: {
    __raw_new_ip_type_req *r = (__raw_new_ip_type_req *)w.req;
    *(r->ret) = __real_raw_new_ip_type(r->type, r->proto);
    break;
}
case __raw_recv: {
    __raw_recv_req *r = (__raw_recv_req *)w.req;
    __real_raw_recv(r->pcb, r->recv, r->recv_arg);
    break;
}
case __raw_remove: {
    __raw_remove_req *r = (__raw_remove_req *)w.req;
    __real_raw_remove(r->pcb);
    break;
}
case __raw_send: {
    __raw_send_req *r = (__raw_send_req *)w.req;
    *(r->ret) = __real_raw_send(r->pcb, r->p);
    break;
}
case __raw_sendto: {
    __raw_sendto_req *r = (__raw_sendto_req *)w.req;
    *(r->ret) = __real_raw_sendto(r->pcb, r->p, r->ipaddr);
    break;
}
case __raw_sendto_if_src: {
    __raw_sendto_if_src_req *r = (__raw_sendto_if_src_req *)w.req;
    *(r->ret) = __real_raw_sendto_if_src(r->pcb, r->p, r->dst_ip, r->netif, r->src_ip);
    break;
}
