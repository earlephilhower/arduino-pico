// Machine generated (makewrapper.py).  DO NOT EDIT
extern int __real_lfs_dir_close(lfs_t * lfs,lfs_dir_t * dir);
extern int __real_lfs_dir_open(lfs_t * lfs,lfs_dir_t * dir,const char * path);
extern int __real_lfs_dir_read(lfs_t * lfs,lfs_dir_t * dir,struct lfs_info * info);
extern int __real_lfs_dir_rewind(lfs_t * lfs,lfs_dir_t * dir);
extern int __real_lfs_dir_seek(lfs_t * lfs,lfs_dir_t * dir,lfs_off_t off);
extern lfs_soff_t __real_lfs_dir_tell(lfs_t * lfs,lfs_dir_t * dir);
extern int __real_lfs_file_close(lfs_t * lfs,lfs_file_t * file);
extern int __real_lfs_file_open(lfs_t * lfs,lfs_file_t * file,const char * path,int flags);
extern int __real_lfs_file_opencfg(lfs_t * lfs,lfs_file_t * file,const char * path,int flags,const struct lfs_file_config * config);
extern lfs_ssize_t __real_lfs_file_read(lfs_t * lfs,lfs_file_t * file,void * buffer,lfs_size_t size);
extern int __real_lfs_file_rewind(lfs_t * lfs,lfs_file_t * file);
extern lfs_soff_t __real_lfs_file_seek(lfs_t * lfs,lfs_file_t * file,lfs_soff_t off,int whence);
extern lfs_soff_t __real_lfs_file_size(lfs_t * lfs,lfs_file_t * file);
extern int __real_lfs_file_sync(lfs_t * lfs,lfs_file_t * file);
extern lfs_soff_t __real_lfs_file_tell(lfs_t * lfs,lfs_file_t * file);
extern int __real_lfs_file_truncate(lfs_t * lfs,lfs_file_t * file,lfs_off_t size);
extern lfs_ssize_t __real_lfs_file_write(lfs_t * lfs,lfs_file_t * file,const void * buffer,lfs_size_t size);
extern int __real_lfs_format(lfs_t * lfs,const struct lfs_config * config);
extern int __real_lfs_fs_gc(lfs_t * lfs);
extern int __real_lfs_fs_grow(lfs_t * lfs,lfs_size_t block_count);
extern int __real_lfs_fs_mkconsistent(lfs_t * lfs);
extern lfs_ssize_t __real_lfs_fs_size(lfs_t * lfs);
extern int __real_lfs_fs_stat(lfs_t * lfs,struct lfs_fsinfo * fsinfo);
extern int __real_lfs_fs_traverse(lfs_t * lfs,int (* cb)(void *,lfs_block_t),void * data);
extern lfs_ssize_t __real_lfs_getattr(lfs_t * lfs,const char * path,uint8_t type,void * buffer,lfs_size_t size);
#ifdef LFS_MIGRATE
extern int __real_lfs_migrate(lfs_t * lfs,const struct lfs_config * cfg);
#endif
extern int __real_lfs_mkdir(lfs_t * lfs,const char * path);
extern int __real_lfs_mount(lfs_t * lfs,const struct lfs_config * config);
extern int __real_lfs_remove(lfs_t * lfs,const char * path);
extern int __real_lfs_removeattr(lfs_t * lfs,const char * path,uint8_t type);
extern int __real_lfs_rename(lfs_t * lfs,const char * oldpath,const char * newpath);
extern int __real_lfs_setattr(lfs_t * lfs,const char * path,uint8_t type,const void * buffer,lfs_size_t size);
extern int __real_lfs_stat(lfs_t * lfs,const char * path,struct lfs_info * info);
extern int __real_lfs_unmount(lfs_t * lfs);
