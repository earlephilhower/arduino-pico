// Machine generated (makewrapper.py).  DO NOT EDIT
typedef struct {
    struct pbuf * p;
    size_t header_size_increment;
    u8_t *ret;
} __pbuf_add_header_req;
typedef struct {
    struct pbuf * p;
    size_t header_size_increment;
    u8_t *ret;
} __pbuf_add_header_force_req;
typedef struct {
    pbuf_layer l;
    u16_t length;
    pbuf_type type;
    struct pbuf * *ret;
} __pbuf_alloc_req;
typedef struct {
    void * payload;
    u16_t length;
    pbuf_type type;
    struct pbuf * *ret;
} __pbuf_alloc_reference_req;
#if LWIP_SUPPORT_CUSTOM_PBUF
typedef struct {
    pbuf_layer l;
    u16_t length;
    pbuf_type type;
    struct pbuf_custom * p;
    void * payload_mem;
    u16_t payload_mem_len;
    struct pbuf * *ret;
} __pbuf_alloced_custom_req;
#endif
typedef struct {
    struct pbuf * head;
    struct pbuf * tail;
} __pbuf_cat_req;
typedef struct {
    struct pbuf * head;
    struct pbuf * tail;
} __pbuf_chain_req;
typedef struct {
    const struct pbuf * p;
    u16_t *ret;
} __pbuf_clen_req;
typedef struct {
    pbuf_layer l;
    pbuf_type type;
    struct pbuf * p;
    struct pbuf * *ret;
} __pbuf_clone_req;
typedef struct {
    struct pbuf * p;
    pbuf_layer layer;
    struct pbuf * *ret;
} __pbuf_coalesce_req;
typedef struct {
    struct pbuf * p_to;
    const struct pbuf * p_from;
    err_t *ret;
} __pbuf_copy_req;
typedef struct {
    const struct pbuf * p;
    void * dataptr;
    u16_t len;
    u16_t offset;
    u16_t *ret;
} __pbuf_copy_partial_req;
typedef struct {
    struct pbuf * p_to;
    const struct pbuf * p_from;
    u16_t copy_len;
    u16_t offset;
    err_t *ret;
} __pbuf_copy_partial_pbuf_req;
typedef struct {
    struct pbuf * p;
    struct pbuf * *ret;
} __pbuf_dechain_req;
#if LWIP_CHECKSUM_ON_COPY
typedef struct {
    struct pbuf * p;
    u16_t start_offset;
    const void * dataptr;
    u16_t len;
    u16_t * chksum;
    err_t *ret;
} __pbuf_fill_chksum_req;
#endif
typedef struct {
    struct pbuf * p;
    u8_t *ret;
} __pbuf_free_req;
typedef struct {
    struct pbuf * q;
    u16_t size;
    struct pbuf * *ret;
} __pbuf_free_header_req;
typedef struct {
    ;
} __pbuf_free_ooseq_req;
typedef struct {
    const struct pbuf * p;
    u16_t offset;
    u8_t *ret;
} __pbuf_get_at_req;
typedef struct {
    const struct pbuf * p;
    void * buffer;
    size_t bufsize;
    u16_t len;
    u16_t offset;
    void * *ret;
} __pbuf_get_contiguous_req;
typedef struct {
    struct pbuf * p;
    s16_t header_size;
    u8_t *ret;
} __pbuf_header_req;
typedef struct {
    struct pbuf * p;
    s16_t header_size;
    u8_t *ret;
} __pbuf_header_force_req;
typedef struct {
    const struct pbuf * p;
    u16_t offset;
    const void * s2;
    u16_t n;
    u16_t *ret;
} __pbuf_memcmp_req;
typedef struct {
    const struct pbuf * p;
    const void * mem;
    u16_t mem_len;
    u16_t start_offset;
    u16_t *ret;
} __pbuf_memfind_req;
typedef struct {
    struct pbuf * p;
    u16_t offset;
    u8_t data;
} __pbuf_put_at_req;
typedef struct {
    struct pbuf * p;
    u16_t size;
} __pbuf_realloc_req;
typedef struct {
    struct pbuf * p;
} __pbuf_ref_req;
typedef struct {
    struct pbuf * p;
    size_t header_size;
    u8_t *ret;
} __pbuf_remove_header_req;
typedef struct {
    struct pbuf * in;
    u16_t in_offset;
    u16_t * out_offset;
    struct pbuf * *ret;
} __pbuf_skip_req;
#if LWIP_TCP && TCP_QUEUE_OOSEQ && LWIP_WND_SCALE
typedef struct {
    struct pbuf * p;
    struct pbuf ** rest;
} __pbuf_split_64k_req;
#endif
typedef struct {
    const struct pbuf * p;
    const char * substr;
    u16_t *ret;
} __pbuf_strstr_req;
typedef struct {
    struct pbuf * buf;
    const void * dataptr;
    u16_t len;
    err_t *ret;
} __pbuf_take_req;
typedef struct {
    struct pbuf * buf;
    const void * dataptr;
    u16_t len;
    u16_t offset;
    err_t *ret;
} __pbuf_take_at_req;
typedef struct {
    const struct pbuf * p;
    u16_t offset;
    int *ret;
} __pbuf_try_get_at_req;
