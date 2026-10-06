// Machine generated (makewrapper.py).  DO NOT EDIT
case __udp_bind: {
    __udp_bind_req *r = (__udp_bind_req *)w.req;
    *(r->ret) = __real_udp_bind(r->pcb, r->ipaddr, r->port);
    break;
}
case __udp_bind_netif: {
    __udp_bind_netif_req *r = (__udp_bind_netif_req *)w.req;
    __real_udp_bind_netif(r->pcb, r->netif);
    break;
}
case __udp_connect: {
    __udp_connect_req *r = (__udp_connect_req *)w.req;
    *(r->ret) = __real_udp_connect(r->pcb, r->ipaddr, r->port);
    break;
}
#if UDP_DEBUG
case __udp_debug_print: {
    __udp_debug_print_req *r = (__udp_debug_print_req *)w.req;
    __real_udp_debug_print(r->udphdr);
    break;
}
#endif
case __udp_disconnect: {
    __udp_disconnect_req *r = (__udp_disconnect_req *)w.req;
    __real_udp_disconnect(r->pcb);
    break;
}
case __udp_init: {
    __real_udp_init();
    break;
}
case __udp_input: {
    __udp_input_req *r = (__udp_input_req *)w.req;
    __real_udp_input(r->p, r->inp);
    break;
}
case __udp_netif_ip_addr_changed: {
    __udp_netif_ip_addr_changed_req *r = (__udp_netif_ip_addr_changed_req *)w.req;
    __real_udp_netif_ip_addr_changed(r->old_addr, r->new_addr);
    break;
}
case __udp_new: {
    __udp_new_req *r = (__udp_new_req *)w.req;
    *(r->ret) = __real_udp_new();
    break;
}
case __udp_new_ip_type: {
    __udp_new_ip_type_req *r = (__udp_new_ip_type_req *)w.req;
    *(r->ret) = __real_udp_new_ip_type(r->type);
    break;
}
case __udp_recv: {
    __udp_recv_req *r = (__udp_recv_req *)w.req;
    __real_udp_recv(r->pcb, r->recv, r->recv_arg);
    break;
}
case __udp_remove: {
    __udp_remove_req *r = (__udp_remove_req *)w.req;
    __real_udp_remove(r->pcb);
    break;
}
case __udp_send: {
    __udp_send_req *r = (__udp_send_req *)w.req;
    *(r->ret) = __real_udp_send(r->pcb, r->p);
    break;
}
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
case __udp_send_chksum: {
    __udp_send_chksum_req *r = (__udp_send_chksum_req *)w.req;
    *(r->ret) = __real_udp_send_chksum(r->pcb, r->p, r->have_chksum, r->chksum);
    break;
}
#endif
case __udp_sendto: {
    __udp_sendto_req *r = (__udp_sendto_req *)w.req;
    *(r->ret) = __real_udp_sendto(r->pcb, r->p, r->dst_ip, r->dst_port);
    break;
}
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
case __udp_sendto_chksum: {
    __udp_sendto_chksum_req *r = (__udp_sendto_chksum_req *)w.req;
    *(r->ret) = __real_udp_sendto_chksum(r->pcb, r->p, r->dst_ip, r->dst_port, r->have_chksum, r->chksum);
    break;
}
#endif
case __udp_sendto_if: {
    __udp_sendto_if_req *r = (__udp_sendto_if_req *)w.req;
    *(r->ret) = __real_udp_sendto_if(r->pcb, r->p, r->dst_ip, r->dst_port, r->netif);
    break;
}
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
case __udp_sendto_if_chksum: {
    __udp_sendto_if_chksum_req *r = (__udp_sendto_if_chksum_req *)w.req;
    *(r->ret) = __real_udp_sendto_if_chksum(r->pcb, r->p, r->dst_ip, r->dst_port, r->netif, r->have_chksum, r->chksum);
    break;
}
#endif
case __udp_sendto_if_src: {
    __udp_sendto_if_src_req *r = (__udp_sendto_if_src_req *)w.req;
    *(r->ret) = __real_udp_sendto_if_src(r->pcb, r->p, r->dst_ip, r->dst_port, r->netif, r->src_ip);
    break;
}
#if LWIP_CHECKSUM_ON_COPY && CHECKSUM_GEN_UDP
case __udp_sendto_if_src_chksum: {
    __udp_sendto_if_src_chksum_req *r = (__udp_sendto_if_src_chksum_req *)w.req;
    *(r->ret) = __real_udp_sendto_if_src_chksum(r->pcb, r->p, r->dst_ip, r->dst_port, r->netif, r->have_chksum, r->chksum, r->src_ip);
    break;
}
#endif
