// Machine generated (makewrapper.py).  DO NOT EDIT
typedef struct {
    lfs_t * lfs;
    lfs_dir_t * dir;
    int *ret;
} __lfs_dir_close_req;
typedef struct {
    lfs_t * lfs;
    lfs_dir_t * dir;
    const char * path;
    int *ret;
} __lfs_dir_open_req;
typedef struct {
    lfs_t * lfs;
    lfs_dir_t * dir;
    struct lfs_info * info;
    int *ret;
} __lfs_dir_read_req;
typedef struct {
    lfs_t * lfs;
    lfs_dir_t * dir;
    int *ret;
} __lfs_dir_rewind_req;
typedef struct {
    lfs_t * lfs;
    lfs_dir_t * dir;
    lfs_off_t off;
    int *ret;
} __lfs_dir_seek_req;
typedef struct {
    lfs_t * lfs;
    lfs_dir_t * dir;
    lfs_soff_t *ret;
} __lfs_dir_tell_req;
typedef struct {
    lfs_t * lfs;
    lfs_file_t * file;
    int *ret;
} __lfs_file_close_req;
typedef struct {
    lfs_t * lfs;
    lfs_file_t * file;
    const char * path;
    int flags;
    int *ret;
} __lfs_file_open_req;
typedef struct {
    lfs_t * lfs;
    lfs_file_t * file;
    const char * path;
    int flags;
    const struct lfs_file_config * config;
    int *ret;
} __lfs_file_opencfg_req;
typedef struct {
    lfs_t * lfs;
    lfs_file_t * file;
    void * buffer;
    lfs_size_t size;
    lfs_ssize_t *ret;
} __lfs_file_read_req;
typedef struct {
    lfs_t * lfs;
    lfs_file_t * file;
    int *ret;
} __lfs_file_rewind_req;
typedef struct {
    lfs_t * lfs;
    lfs_file_t * file;
    lfs_soff_t off;
    int whence;
    lfs_soff_t *ret;
} __lfs_file_seek_req;
typedef struct {
    lfs_t * lfs;
    lfs_file_t * file;
    lfs_soff_t *ret;
} __lfs_file_size_req;
typedef struct {
    lfs_t * lfs;
    lfs_file_t * file;
    int *ret;
} __lfs_file_sync_req;
typedef struct {
    lfs_t * lfs;
    lfs_file_t * file;
    lfs_soff_t *ret;
} __lfs_file_tell_req;
typedef struct {
    lfs_t * lfs;
    lfs_file_t * file;
    lfs_off_t size;
    int *ret;
} __lfs_file_truncate_req;
typedef struct {
    lfs_t * lfs;
    lfs_file_t * file;
    const void * buffer;
    lfs_size_t size;
    lfs_ssize_t *ret;
} __lfs_file_write_req;
typedef struct {
    lfs_t * lfs;
    const struct lfs_config * config;
    int *ret;
} __lfs_format_req;
typedef struct {
    lfs_t * lfs;
    int *ret;
} __lfs_fs_gc_req;
typedef struct {
    lfs_t * lfs;
    lfs_size_t block_count;
    int *ret;
} __lfs_fs_grow_req;
typedef struct {
    lfs_t * lfs;
    int *ret;
} __lfs_fs_mkconsistent_req;
typedef struct {
    lfs_t * lfs;
    lfs_ssize_t *ret;
} __lfs_fs_size_req;
typedef struct {
    lfs_t * lfs;
    struct lfs_fsinfo * fsinfo;
    int *ret;
} __lfs_fs_stat_req;
typedef struct {
    lfs_t * lfs;
    int (* cb)(void *,lfs_block_t);
    void * data;
    int *ret;
} __lfs_fs_traverse_req;
typedef struct {
    lfs_t * lfs;
    const char * path;
    uint8_t type;
    void * buffer;
    lfs_size_t size;
    lfs_ssize_t *ret;
} __lfs_getattr_req;
#ifdef LFS_MIGRATE
typedef struct {
    lfs_t * lfs;
    const struct lfs_config * cfg;
    int *ret;
} __lfs_migrate_req;
#endif
typedef struct {
    lfs_t * lfs;
    const char * path;
    int *ret;
} __lfs_mkdir_req;
typedef struct {
    lfs_t * lfs;
    const struct lfs_config * config;
    int *ret;
} __lfs_mount_req;
typedef struct {
    lfs_t * lfs;
    const char * path;
    int *ret;
} __lfs_remove_req;
typedef struct {
    lfs_t * lfs;
    const char * path;
    uint8_t type;
    int *ret;
} __lfs_removeattr_req;
typedef struct {
    lfs_t * lfs;
    const char * oldpath;
    const char * newpath;
    int *ret;
} __lfs_rename_req;
typedef struct {
    lfs_t * lfs;
    const char * path;
    uint8_t type;
    const void * buffer;
    lfs_size_t size;
    int *ret;
} __lfs_setattr_req;
typedef struct {
    lfs_t * lfs;
    const char * path;
    struct lfs_info * info;
    int *ret;
} __lfs_stat_req;
typedef struct {
    lfs_t * lfs;
    int *ret;
} __lfs_unmount_req;
