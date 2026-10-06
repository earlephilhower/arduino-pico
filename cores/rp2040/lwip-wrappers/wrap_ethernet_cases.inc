// Machine generated (makewrapper.py).  DO NOT EDIT
case __ethernet_input: {
    __ethernet_input_req *r = (__ethernet_input_req *)w.req;
    *(r->ret) = __real_ethernet_input(r->p, r->netif);
    break;
}
case __ethernet_output: {
    __ethernet_output_req *r = (__ethernet_output_req *)w.req;
    *(r->ret) = __real_ethernet_output(r->netif, r->p, r->src, r->dst, r->eth_type);
    break;
}
