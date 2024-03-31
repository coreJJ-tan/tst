suguoxu@sgx:~/linux/my/pro/bsp/kernel/linux/fs$ grep -d skip __init *
aio.c:static int __init aio_setup(void)
aio.c:__initcall(aio_setup);
anon_inodes.c:static int __init anon_inode_init(void)
binfmt_aout.c:static int __init init_aout_binfmt(void)
binfmt_elf.c:static int __init init_elf_binfmt(void)
binfmt_elf_fdpic.c:static int __init init_elf_fdpic_binfmt(void)
binfmt_em86.c:static int __init init_em86_binfmt(void)
binfmt_flat.c:static int __init init_flat_binfmt(void)
binfmt_misc.c:static int __init init_misc_binfmt(void)
binfmt_script.c:static int __init init_script_binfmt(void)
block_dev.c:void __init bdev_cache_init(void)
buffer.c:void __init buffer_init(void)
char_dev.c:void __init chrdev_init(void)
compat_ioctl.c:static int __init init_sys32_ioctl_cmp(const void *p, const void *q)
compat_ioctl.c:static int __init init_sys32_ioctl(void)
compat_ioctl.c:__initcall(init_sys32_ioctl);
dcache.c:static __initdata unsigned long dhash_entries;
dcache.c:static int __init set_dhash_entries(char *str)
dcache.c:static void __init dcache_init_early(void)
dcache.c:static void __init dcache_init(void)
dcache.c:void __init vfs_caches_init_early(void)
dcache.c:void __init vfs_caches_init(unsigned long mempages)
direct-io.c:static __init int dio_init(void)
eventpoll.c:static int __init eventpoll_init(void)
fcntl.c:static int __init fcntl_init(void)
filesystems.c:int __init get_filesystem_list(char *buf)
filesystems.c:static int __init proc_filesystems_init(void)
file_table.c:void __init files_init(unsigned long mempages)
fs-writeback.c:static int __init start_dirtytime_writeback(void)
fs-writeback.c:__initcall(start_dirtytime_writeback);
inode.c:static __initdata unsigned long ihash_entries;
inode.c:static int __init set_ihash_entries(char *str)
inode.c:void __init inode_init_early(void)
inode.c:void __init inode_init(void)
internal.h:extern void __init bdev_cache_init(void);
internal.h:extern void __init chrdev_init(void);
internal.h:extern void __init mnt_init(void);
locks.c:static int __init proc_locks_init(void)
locks.c:static int __init filelock_init(void)
mbcache.c:static int __init init_mbcache(void)
namespace.c:static __initdata unsigned long mhash_entries;
namespace.c:static int __init set_mhash_entries(char *str)
namespace.c:static __initdata unsigned long mphash_entries;
namespace.c:static int __init set_mphash_entries(char *str)
namespace.c:static void __init init_mount_tree(void)
namespace.c:void __init mnt_init(void)
nsfs.c:void __init nsfs_init(void)
pipe.c:static int __init init_pipe_fs(void)
suguoxu@sgx:~/linux/my/pro/bsp/kernel/linux/fs$
 