// Machine generated (makewrapper.py).  DO NOT EDIT
err_t __wrap_ethernet_input(struct pbuf * p,struct netif * netif) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __ethernet_input_req req = { p, netif, &ret };
        __lwip(__ethernet_input, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_ethernet_input(p, netif);
}

err_t __wrap_ethernet_output(struct netif * netif,struct pbuf * p,const struct eth_addr * src,const struct eth_addr * dst,u16_t eth_type) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __ethernet_output_req req = { netif, p, src, dst, eth_type, &ret };
        __lwip(__ethernet_output, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_ethernet_output(netif, p, src, dst, eth_type);
}

