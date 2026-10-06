// Machine generated (makewrapper.py).  DO NOT EDIT
u8_t __wrap_pbuf_add_header(struct pbuf * p,size_t header_size_increment) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __pbuf_add_header_req req = { p, header_size_increment, &ret };
        __lwip(__pbuf_add_header, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_add_header(p, header_size_increment);
}

u8_t __wrap_pbuf_add_header_force(struct pbuf * p,size_t header_size_increment) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __pbuf_add_header_force_req req = { p, header_size_increment, &ret };
        __lwip(__pbuf_add_header_force, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_add_header_force(p, header_size_increment);
}

struct pbuf * __wrap_pbuf_alloc(pbuf_layer l,u16_t length,pbuf_type type) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct pbuf * ret;
        __pbuf_alloc_req req = { l, length, type, &ret };
        __lwip(__pbuf_alloc, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_alloc(l, length, type);
}

struct pbuf * __wrap_pbuf_alloc_reference(void * payload,u16_t length,pbuf_type type) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct pbuf * ret;
        __pbuf_alloc_reference_req req = { payload, length, type, &ret };
        __lwip(__pbuf_alloc_reference, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_alloc_reference(payload, length, type);
}

#if LWIP_SUPPORT_CUSTOM_PBUF
struct pbuf * __wrap_pbuf_alloced_custom(pbuf_layer l,u16_t length,pbuf_type type,struct pbuf_custom * p,void * payload_mem,u16_t payload_mem_len) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct pbuf * ret;
        __pbuf_alloced_custom_req req = { l, length, type, p, payload_mem, payload_mem_len, &ret };
        __lwip(__pbuf_alloced_custom, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_alloced_custom(l, length, type, p, payload_mem, payload_mem_len);
}

#endif
void __wrap_pbuf_cat(struct pbuf * head,struct pbuf * tail) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __pbuf_cat_req req = { head, tail };
        __lwip(__pbuf_cat, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_pbuf_cat(head, tail);
}

void __wrap_pbuf_chain(struct pbuf * head,struct pbuf * tail) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __pbuf_chain_req req = { head, tail };
        __lwip(__pbuf_chain, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_pbuf_chain(head, tail);
}

u16_t __wrap_pbuf_clen(const struct pbuf * p) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u16_t ret;
        __pbuf_clen_req req = { p, &ret };
        __lwip(__pbuf_clen, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_clen(p);
}

struct pbuf * __wrap_pbuf_clone(pbuf_layer l,pbuf_type type,struct pbuf * p) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct pbuf * ret;
        __pbuf_clone_req req = { l, type, p, &ret };
        __lwip(__pbuf_clone, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_clone(l, type, p);
}

struct pbuf * __wrap_pbuf_coalesce(struct pbuf * p,pbuf_layer layer) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct pbuf * ret;
        __pbuf_coalesce_req req = { p, layer, &ret };
        __lwip(__pbuf_coalesce, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_coalesce(p, layer);
}

err_t __wrap_pbuf_copy(struct pbuf * p_to,const struct pbuf * p_from) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __pbuf_copy_req req = { p_to, p_from, &ret };
        __lwip(__pbuf_copy, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_copy(p_to, p_from);
}

u16_t __wrap_pbuf_copy_partial(const struct pbuf * p,void * dataptr,u16_t len,u16_t offset) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u16_t ret;
        __pbuf_copy_partial_req req = { p, dataptr, len, offset, &ret };
        __lwip(__pbuf_copy_partial, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_copy_partial(p, dataptr, len, offset);
}

err_t __wrap_pbuf_copy_partial_pbuf(struct pbuf * p_to,const struct pbuf * p_from,u16_t copy_len,u16_t offset) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __pbuf_copy_partial_pbuf_req req = { p_to, p_from, copy_len, offset, &ret };
        __lwip(__pbuf_copy_partial_pbuf, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_copy_partial_pbuf(p_to, p_from, copy_len, offset);
}

struct pbuf * __wrap_pbuf_dechain(struct pbuf * p) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct pbuf * ret;
        __pbuf_dechain_req req = { p, &ret };
        __lwip(__pbuf_dechain, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_dechain(p);
}

#if LWIP_CHECKSUM_ON_COPY
err_t __wrap_pbuf_fill_chksum(struct pbuf * p,u16_t start_offset,const void * dataptr,u16_t len,u16_t * chksum) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __pbuf_fill_chksum_req req = { p, start_offset, dataptr, len, chksum, &ret };
        __lwip(__pbuf_fill_chksum, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_fill_chksum(p, start_offset, dataptr, len, chksum);
}

#endif
u8_t __wrap_pbuf_free(struct pbuf * p) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __pbuf_free_req req = { p, &ret };
        __lwip(__pbuf_free, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_free(p);
}

struct pbuf * __wrap_pbuf_free_header(struct pbuf * q,u16_t size) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct pbuf * ret;
        __pbuf_free_header_req req = { q, size, &ret };
        __lwip(__pbuf_free_header, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_free_header(q, size);
}

void __wrap_pbuf_free_ooseq() {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __pbuf_free_ooseq_req req = {  };
        __lwip(__pbuf_free_ooseq, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_pbuf_free_ooseq();
}

u8_t __wrap_pbuf_get_at(const struct pbuf * p,u16_t offset) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __pbuf_get_at_req req = { p, offset, &ret };
        __lwip(__pbuf_get_at, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_get_at(p, offset);
}

void * __wrap_pbuf_get_contiguous(const struct pbuf * p,void * buffer,size_t bufsize,u16_t len,u16_t offset) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        void * ret;
        __pbuf_get_contiguous_req req = { p, buffer, bufsize, len, offset, &ret };
        __lwip(__pbuf_get_contiguous, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_get_contiguous(p, buffer, bufsize, len, offset);
}

u8_t __wrap_pbuf_header(struct pbuf * p,s16_t header_size) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __pbuf_header_req req = { p, header_size, &ret };
        __lwip(__pbuf_header, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_header(p, header_size);
}

u8_t __wrap_pbuf_header_force(struct pbuf * p,s16_t header_size) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __pbuf_header_force_req req = { p, header_size, &ret };
        __lwip(__pbuf_header_force, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_header_force(p, header_size);
}

u16_t __wrap_pbuf_memcmp(const struct pbuf * p,u16_t offset,const void * s2,u16_t n) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u16_t ret;
        __pbuf_memcmp_req req = { p, offset, s2, n, &ret };
        __lwip(__pbuf_memcmp, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_memcmp(p, offset, s2, n);
}

u16_t __wrap_pbuf_memfind(const struct pbuf * p,const void * mem,u16_t mem_len,u16_t start_offset) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u16_t ret;
        __pbuf_memfind_req req = { p, mem, mem_len, start_offset, &ret };
        __lwip(__pbuf_memfind, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_memfind(p, mem, mem_len, start_offset);
}

void __wrap_pbuf_put_at(struct pbuf * p,u16_t offset,u8_t data) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __pbuf_put_at_req req = { p, offset, data };
        __lwip(__pbuf_put_at, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_pbuf_put_at(p, offset, data);
}

void __wrap_pbuf_realloc(struct pbuf * p,u16_t size) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __pbuf_realloc_req req = { p, size };
        __lwip(__pbuf_realloc, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_pbuf_realloc(p, size);
}

void __wrap_pbuf_ref(struct pbuf * p) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __pbuf_ref_req req = { p };
        __lwip(__pbuf_ref, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_pbuf_ref(p);
}

u8_t __wrap_pbuf_remove_header(struct pbuf * p,size_t header_size) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u8_t ret;
        __pbuf_remove_header_req req = { p, header_size, &ret };
        __lwip(__pbuf_remove_header, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_remove_header(p, header_size);
}

struct pbuf * __wrap_pbuf_skip(struct pbuf * in,u16_t in_offset,u16_t * out_offset) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        struct pbuf * ret;
        __pbuf_skip_req req = { in, in_offset, out_offset, &ret };
        __lwip(__pbuf_skip, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_skip(in, in_offset, out_offset);
}

#if LWIP_TCP && TCP_QUEUE_OOSEQ && LWIP_WND_SCALE
void __wrap_pbuf_split_64k(struct pbuf * p,struct pbuf ** rest) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        __pbuf_split_64k_req req = { p, rest };
        __lwip(__pbuf_split_64k, &req);
        return;
    }
#endif
    LWIPMutex m;
    __real_pbuf_split_64k(p, rest);
}

#endif
u16_t __wrap_pbuf_strstr(const struct pbuf * p,const char * substr) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        u16_t ret;
        __pbuf_strstr_req req = { p, substr, &ret };
        __lwip(__pbuf_strstr, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_strstr(p, substr);
}

err_t __wrap_pbuf_take(struct pbuf * buf,const void * dataptr,u16_t len) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __pbuf_take_req req = { buf, dataptr, len, &ret };
        __lwip(__pbuf_take, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_take(buf, dataptr, len);
}

err_t __wrap_pbuf_take_at(struct pbuf * buf,const void * dataptr,u16_t len,u16_t offset) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        err_t ret;
        __pbuf_take_at_req req = { buf, dataptr, len, offset, &ret };
        __lwip(__pbuf_take_at, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_take_at(buf, dataptr, len, offset);
}

int __wrap_pbuf_try_get_at(const struct pbuf * p,u16_t offset) {
#ifdef __FREERTOS
    if (!__isLWIPThread()) {
        int ret;
        __pbuf_try_get_at_req req = { p, offset, &ret };
        __lwip(__pbuf_try_get_at, &req);
        return ret;
    }
#endif
    LWIPMutex m;
    return __real_pbuf_try_get_at(p, offset);
}

