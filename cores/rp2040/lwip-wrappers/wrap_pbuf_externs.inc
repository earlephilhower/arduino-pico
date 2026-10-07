// Machine generated (makewrapper.py).  DO NOT EDIT
extern u8_t __real_pbuf_add_header(struct pbuf * p,size_t header_size_increment);
extern u8_t __real_pbuf_add_header_force(struct pbuf * p,size_t header_size_increment);
extern struct pbuf * __real_pbuf_alloc(pbuf_layer l,u16_t length,pbuf_type type);
extern struct pbuf * __real_pbuf_alloc_reference(void * payload,u16_t length,pbuf_type type);
#if LWIP_SUPPORT_CUSTOM_PBUF
extern struct pbuf * __real_pbuf_alloced_custom(pbuf_layer l,u16_t length,pbuf_type type,struct pbuf_custom * p,void * payload_mem,u16_t payload_mem_len);
#endif
extern void __real_pbuf_cat(struct pbuf * head,struct pbuf * tail);
extern void __real_pbuf_chain(struct pbuf * head,struct pbuf * tail);
extern u16_t __real_pbuf_clen(const struct pbuf * p);
extern struct pbuf * __real_pbuf_clone(pbuf_layer l,pbuf_type type,struct pbuf * p);
extern struct pbuf * __real_pbuf_coalesce(struct pbuf * p,pbuf_layer layer);
extern err_t __real_pbuf_copy(struct pbuf * p_to,const struct pbuf * p_from);
extern u16_t __real_pbuf_copy_partial(const struct pbuf * p,void * dataptr,u16_t len,u16_t offset);
extern err_t __real_pbuf_copy_partial_pbuf(struct pbuf * p_to,const struct pbuf * p_from,u16_t copy_len,u16_t offset);
extern struct pbuf * __real_pbuf_dechain(struct pbuf * p);
#if LWIP_CHECKSUM_ON_COPY
extern err_t __real_pbuf_fill_chksum(struct pbuf * p,u16_t start_offset,const void * dataptr,u16_t len,u16_t * chksum);
#endif
extern u8_t __real_pbuf_free(struct pbuf * p);
extern struct pbuf * __real_pbuf_free_header(struct pbuf * q,u16_t size);
extern void __real_pbuf_free_ooseq();
extern u8_t __real_pbuf_get_at(const struct pbuf * p,u16_t offset);
extern void * __real_pbuf_get_contiguous(const struct pbuf * p,void * buffer,size_t bufsize,u16_t len,u16_t offset);
extern u8_t __real_pbuf_header(struct pbuf * p,s16_t header_size);
extern u8_t __real_pbuf_header_force(struct pbuf * p,s16_t header_size);
extern u16_t __real_pbuf_memcmp(const struct pbuf * p,u16_t offset,const void * s2,u16_t n);
extern u16_t __real_pbuf_memfind(const struct pbuf * p,const void * mem,u16_t mem_len,u16_t start_offset);
extern void __real_pbuf_put_at(struct pbuf * p,u16_t offset,u8_t data);
extern void __real_pbuf_realloc(struct pbuf * p,u16_t size);
extern void __real_pbuf_ref(struct pbuf * p);
extern u8_t __real_pbuf_remove_header(struct pbuf * p,size_t header_size);
extern struct pbuf * __real_pbuf_skip(struct pbuf * in,u16_t in_offset,u16_t * out_offset);
#if LWIP_TCP && TCP_QUEUE_OOSEQ && LWIP_WND_SCALE
extern void __real_pbuf_split_64k(struct pbuf * p,struct pbuf ** rest);
#endif
extern u16_t __real_pbuf_strstr(const struct pbuf * p,const char * substr);
extern err_t __real_pbuf_take(struct pbuf * buf,const void * dataptr,u16_t len);
extern err_t __real_pbuf_take_at(struct pbuf * buf,const void * dataptr,u16_t len,u16_t offset);
extern int __real_pbuf_try_get_at(const struct pbuf * p,u16_t offset);
