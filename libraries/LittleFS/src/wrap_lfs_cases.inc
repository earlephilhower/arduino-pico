// Machine generated (makewrapper.py).  DO NOT EDIT
case __lfs_dir_close: {
    __lfs_dir_close_req *r = (__lfs_dir_close_req *)w.req;
    *(r->ret) = __real_lfs_dir_close(r->lfs, r->dir);
    break;
}
case __lfs_dir_open: {
    __lfs_dir_open_req *r = (__lfs_dir_open_req *)w.req;
    *(r->ret) = __real_lfs_dir_open(r->lfs, r->dir, r->path);
    break;
}
case __lfs_dir_read: {
    __lfs_dir_read_req *r = (__lfs_dir_read_req *)w.req;
    *(r->ret) = __real_lfs_dir_read(r->lfs, r->dir, r->info);
    break;
}
case __lfs_dir_rewind: {
    __lfs_dir_rewind_req *r = (__lfs_dir_rewind_req *)w.req;
    *(r->ret) = __real_lfs_dir_rewind(r->lfs, r->dir);
    break;
}
case __lfs_dir_seek: {
    __lfs_dir_seek_req *r = (__lfs_dir_seek_req *)w.req;
    *(r->ret) = __real_lfs_dir_seek(r->lfs, r->dir, r->off);
    break;
}
case __lfs_dir_tell: {
    __lfs_dir_tell_req *r = (__lfs_dir_tell_req *)w.req;
    *(r->ret) = __real_lfs_dir_tell(r->lfs, r->dir);
    break;
}
case __lfs_file_close: {
    __lfs_file_close_req *r = (__lfs_file_close_req *)w.req;
    *(r->ret) = __real_lfs_file_close(r->lfs, r->file);
    break;
}
case __lfs_file_open: {
    __lfs_file_open_req *r = (__lfs_file_open_req *)w.req;
    *(r->ret) = __real_lfs_file_open(r->lfs, r->file, r->path, r->flags);
    break;
}
case __lfs_file_opencfg: {
    __lfs_file_opencfg_req *r = (__lfs_file_opencfg_req *)w.req;
    *(r->ret) = __real_lfs_file_opencfg(r->lfs, r->file, r->path, r->flags, r->config);
    break;
}
case __lfs_file_read: {
    __lfs_file_read_req *r = (__lfs_file_read_req *)w.req;
    *(r->ret) = __real_lfs_file_read(r->lfs, r->file, r->buffer, r->size);
    break;
}
case __lfs_file_rewind: {
    __lfs_file_rewind_req *r = (__lfs_file_rewind_req *)w.req;
    *(r->ret) = __real_lfs_file_rewind(r->lfs, r->file);
    break;
}
case __lfs_file_seek: {
    __lfs_file_seek_req *r = (__lfs_file_seek_req *)w.req;
    *(r->ret) = __real_lfs_file_seek(r->lfs, r->file, r->off, r->whence);
    break;
}
case __lfs_file_size: {
    __lfs_file_size_req *r = (__lfs_file_size_req *)w.req;
    *(r->ret) = __real_lfs_file_size(r->lfs, r->file);
    break;
}
case __lfs_file_sync: {
    __lfs_file_sync_req *r = (__lfs_file_sync_req *)w.req;
    *(r->ret) = __real_lfs_file_sync(r->lfs, r->file);
    break;
}
case __lfs_file_tell: {
    __lfs_file_tell_req *r = (__lfs_file_tell_req *)w.req;
    *(r->ret) = __real_lfs_file_tell(r->lfs, r->file);
    break;
}
case __lfs_file_truncate: {
    __lfs_file_truncate_req *r = (__lfs_file_truncate_req *)w.req;
    *(r->ret) = __real_lfs_file_truncate(r->lfs, r->file, r->size);
    break;
}
case __lfs_file_write: {
    __lfs_file_write_req *r = (__lfs_file_write_req *)w.req;
    *(r->ret) = __real_lfs_file_write(r->lfs, r->file, r->buffer, r->size);
    break;
}
case __lfs_format: {
    __lfs_format_req *r = (__lfs_format_req *)w.req;
    *(r->ret) = __real_lfs_format(r->lfs, r->config);
    break;
}
case __lfs_fs_gc: {
    __lfs_fs_gc_req *r = (__lfs_fs_gc_req *)w.req;
    *(r->ret) = __real_lfs_fs_gc(r->lfs);
    break;
}
case __lfs_fs_grow: {
    __lfs_fs_grow_req *r = (__lfs_fs_grow_req *)w.req;
    *(r->ret) = __real_lfs_fs_grow(r->lfs, r->block_count);
    break;
}
case __lfs_fs_mkconsistent: {
    __lfs_fs_mkconsistent_req *r = (__lfs_fs_mkconsistent_req *)w.req;
    *(r->ret) = __real_lfs_fs_mkconsistent(r->lfs);
    break;
}
case __lfs_fs_size: {
    __lfs_fs_size_req *r = (__lfs_fs_size_req *)w.req;
    *(r->ret) = __real_lfs_fs_size(r->lfs);
    break;
}
case __lfs_fs_stat: {
    __lfs_fs_stat_req *r = (__lfs_fs_stat_req *)w.req;
    *(r->ret) = __real_lfs_fs_stat(r->lfs, r->fsinfo);
    break;
}
case __lfs_fs_traverse: {
    __lfs_fs_traverse_req *r = (__lfs_fs_traverse_req *)w.req;
    *(r->ret) = __real_lfs_fs_traverse(r->lfs, r->cb, r->data);
    break;
}
case __lfs_getattr: {
    __lfs_getattr_req *r = (__lfs_getattr_req *)w.req;
    *(r->ret) = __real_lfs_getattr(r->lfs, r->path, r->type, r->buffer, r->size);
    break;
}
#ifdef LFS_MIGRATE
case __lfs_migrate: {
    __lfs_migrate_req *r = (__lfs_migrate_req *)w.req;
    *(r->ret) = __real_lfs_migrate(r->lfs, r->cfg);
    break;
}
#endif
case __lfs_mkdir: {
    __lfs_mkdir_req *r = (__lfs_mkdir_req *)w.req;
    *(r->ret) = __real_lfs_mkdir(r->lfs, r->path);
    break;
}
case __lfs_mount: {
    __lfs_mount_req *r = (__lfs_mount_req *)w.req;
    *(r->ret) = __real_lfs_mount(r->lfs, r->config);
    break;
}
case __lfs_remove: {
    __lfs_remove_req *r = (__lfs_remove_req *)w.req;
    *(r->ret) = __real_lfs_remove(r->lfs, r->path);
    break;
}
case __lfs_removeattr: {
    __lfs_removeattr_req *r = (__lfs_removeattr_req *)w.req;
    *(r->ret) = __real_lfs_removeattr(r->lfs, r->path, r->type);
    break;
}
case __lfs_rename: {
    __lfs_rename_req *r = (__lfs_rename_req *)w.req;
    *(r->ret) = __real_lfs_rename(r->lfs, r->oldpath, r->newpath);
    break;
}
case __lfs_setattr: {
    __lfs_setattr_req *r = (__lfs_setattr_req *)w.req;
    *(r->ret) = __real_lfs_setattr(r->lfs, r->path, r->type, r->buffer, r->size);
    break;
}
case __lfs_stat: {
    __lfs_stat_req *r = (__lfs_stat_req *)w.req;
    *(r->ret) = __real_lfs_stat(r->lfs, r->path, r->info);
    break;
}
case __lfs_unmount: {
    __lfs_unmount_req *r = (__lfs_unmount_req *)w.req;
    *(r->ret) = __real_lfs_unmount(r->lfs);
    break;
}
