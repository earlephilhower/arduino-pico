// Machine generated (makewrapper.py).  DO NOT EDIT
int __wrap_lfs_dir_close(lfs_t * lfs,lfs_dir_t * dir) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_dir_close_req req = { lfs, dir, &ret };
        __lfs(__lfs_dir_close, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_dir_close(lfs, dir);
}

int __wrap_lfs_dir_open(lfs_t * lfs,lfs_dir_t * dir,const char * path) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_dir_open_req req = { lfs, dir, path, &ret };
        __lfs(__lfs_dir_open, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_dir_open(lfs, dir, path);
}

int __wrap_lfs_dir_read(lfs_t * lfs,lfs_dir_t * dir,struct lfs_info * info) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_dir_read_req req = { lfs, dir, info, &ret };
        __lfs(__lfs_dir_read, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_dir_read(lfs, dir, info);
}

int __wrap_lfs_dir_rewind(lfs_t * lfs,lfs_dir_t * dir) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_dir_rewind_req req = { lfs, dir, &ret };
        __lfs(__lfs_dir_rewind, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_dir_rewind(lfs, dir);
}

int __wrap_lfs_dir_seek(lfs_t * lfs,lfs_dir_t * dir,lfs_off_t off) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_dir_seek_req req = { lfs, dir, off, &ret };
        __lfs(__lfs_dir_seek, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_dir_seek(lfs, dir, off);
}

lfs_soff_t __wrap_lfs_dir_tell(lfs_t * lfs,lfs_dir_t * dir) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        lfs_soff_t ret;
        __lfs_dir_tell_req req = { lfs, dir, &ret };
        __lfs(__lfs_dir_tell, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_dir_tell(lfs, dir);
}

int __wrap_lfs_file_close(lfs_t * lfs,lfs_file_t * file) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_file_close_req req = { lfs, file, &ret };
        __lfs(__lfs_file_close, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_file_close(lfs, file);
}

int __wrap_lfs_file_open(lfs_t * lfs,lfs_file_t * file,const char * path,int flags) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_file_open_req req = { lfs, file, path, flags, &ret };
        __lfs(__lfs_file_open, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_file_open(lfs, file, path, flags);
}

int __wrap_lfs_file_opencfg(lfs_t * lfs,lfs_file_t * file,const char * path,int flags,const struct lfs_file_config * config) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_file_opencfg_req req = { lfs, file, path, flags, config, &ret };
        __lfs(__lfs_file_opencfg, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_file_opencfg(lfs, file, path, flags, config);
}

lfs_ssize_t __wrap_lfs_file_read(lfs_t * lfs,lfs_file_t * file,void * buffer,lfs_size_t size) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        lfs_ssize_t ret;
        __lfs_file_read_req req = { lfs, file, buffer, size, &ret };
        __lfs(__lfs_file_read, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_file_read(lfs, file, buffer, size);
}

int __wrap_lfs_file_rewind(lfs_t * lfs,lfs_file_t * file) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_file_rewind_req req = { lfs, file, &ret };
        __lfs(__lfs_file_rewind, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_file_rewind(lfs, file);
}

lfs_soff_t __wrap_lfs_file_seek(lfs_t * lfs,lfs_file_t * file,lfs_soff_t off,int whence) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        lfs_soff_t ret;
        __lfs_file_seek_req req = { lfs, file, off, whence, &ret };
        __lfs(__lfs_file_seek, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_file_seek(lfs, file, off, whence);
}

lfs_soff_t __wrap_lfs_file_size(lfs_t * lfs,lfs_file_t * file) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        lfs_soff_t ret;
        __lfs_file_size_req req = { lfs, file, &ret };
        __lfs(__lfs_file_size, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_file_size(lfs, file);
}

int __wrap_lfs_file_sync(lfs_t * lfs,lfs_file_t * file) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_file_sync_req req = { lfs, file, &ret };
        __lfs(__lfs_file_sync, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_file_sync(lfs, file);
}

lfs_soff_t __wrap_lfs_file_tell(lfs_t * lfs,lfs_file_t * file) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        lfs_soff_t ret;
        __lfs_file_tell_req req = { lfs, file, &ret };
        __lfs(__lfs_file_tell, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_file_tell(lfs, file);
}

int __wrap_lfs_file_truncate(lfs_t * lfs,lfs_file_t * file,lfs_off_t size) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_file_truncate_req req = { lfs, file, size, &ret };
        __lfs(__lfs_file_truncate, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_file_truncate(lfs, file, size);
}

lfs_ssize_t __wrap_lfs_file_write(lfs_t * lfs,lfs_file_t * file,const void * buffer,lfs_size_t size) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        lfs_ssize_t ret;
        __lfs_file_write_req req = { lfs, file, buffer, size, &ret };
        __lfs(__lfs_file_write, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_file_write(lfs, file, buffer, size);
}

int __wrap_lfs_format(lfs_t * lfs,const struct lfs_config * config) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_format_req req = { lfs, config, &ret };
        __lfs(__lfs_format, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_format(lfs, config);
}

int __wrap_lfs_fs_gc(lfs_t * lfs) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_fs_gc_req req = { lfs, &ret };
        __lfs(__lfs_fs_gc, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_fs_gc(lfs);
}

int __wrap_lfs_fs_grow(lfs_t * lfs,lfs_size_t block_count) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_fs_grow_req req = { lfs, block_count, &ret };
        __lfs(__lfs_fs_grow, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_fs_grow(lfs, block_count);
}

int __wrap_lfs_fs_mkconsistent(lfs_t * lfs) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_fs_mkconsistent_req req = { lfs, &ret };
        __lfs(__lfs_fs_mkconsistent, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_fs_mkconsistent(lfs);
}

lfs_ssize_t __wrap_lfs_fs_size(lfs_t * lfs) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        lfs_ssize_t ret;
        __lfs_fs_size_req req = { lfs, &ret };
        __lfs(__lfs_fs_size, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_fs_size(lfs);
}

int __wrap_lfs_fs_stat(lfs_t * lfs,struct lfs_fsinfo * fsinfo) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_fs_stat_req req = { lfs, fsinfo, &ret };
        __lfs(__lfs_fs_stat, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_fs_stat(lfs, fsinfo);
}

int __wrap_lfs_fs_traverse(lfs_t * lfs,int (* cb)(void *,lfs_block_t),void * data) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_fs_traverse_req req = { lfs, cb, data, &ret };
        __lfs(__lfs_fs_traverse, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_fs_traverse(lfs, cb, data);
}

lfs_ssize_t __wrap_lfs_getattr(lfs_t * lfs,const char * path,uint8_t type,void * buffer,lfs_size_t size) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        lfs_ssize_t ret;
        __lfs_getattr_req req = { lfs, path, type, buffer, size, &ret };
        __lfs(__lfs_getattr, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_getattr(lfs, path, type, buffer, size);
}

#ifdef LFS_MIGRATE
int __wrap_lfs_migrate(lfs_t * lfs,const struct lfs_config * cfg) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_migrate_req req = { lfs, cfg, &ret };
        __lfs(__lfs_migrate, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_migrate(lfs, cfg);
}
#endif

int __wrap_lfs_mkdir(lfs_t * lfs,const char * path) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_mkdir_req req = { lfs, path, &ret };
        __lfs(__lfs_mkdir, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_mkdir(lfs, path);
}

int __wrap_lfs_mount(lfs_t * lfs,const struct lfs_config * config) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_mount_req req = { lfs, config, &ret };
        __lfs(__lfs_mount, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_mount(lfs, config);
}

int __wrap_lfs_remove(lfs_t * lfs,const char * path) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_remove_req req = { lfs, path, &ret };
        __lfs(__lfs_remove, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_remove(lfs, path);
}

int __wrap_lfs_removeattr(lfs_t * lfs,const char * path,uint8_t type) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_removeattr_req req = { lfs, path, type, &ret };
        __lfs(__lfs_removeattr, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_removeattr(lfs, path, type);
}

int __wrap_lfs_rename(lfs_t * lfs,const char * oldpath,const char * newpath) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_rename_req req = { lfs, oldpath, newpath, &ret };
        __lfs(__lfs_rename, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_rename(lfs, oldpath, newpath);
}

int __wrap_lfs_setattr(lfs_t * lfs,const char * path,uint8_t type,const void * buffer,lfs_size_t size) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_setattr_req req = { lfs, path, type, buffer, size, &ret };
        __lfs(__lfs_setattr, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_setattr(lfs, path, type, buffer, size);
}

int __wrap_lfs_stat(lfs_t * lfs,const char * path,struct lfs_info * info) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_stat_req req = { lfs, path, info, &ret };
        __lfs(__lfs_stat, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_stat(lfs, path, info);
}

int __wrap_lfs_unmount(lfs_t * lfs) {
#ifdef __FREERTOS
    if (!__isLFSThread()) {
        int ret;
        __lfs_unmount_req req = { lfs, &ret };
        __lfs(__lfs_unmount, &req);
        return ret;
    }
#endif
    LFSMutex m;
    return __real_lfs_unmount(lfs);
}

