suguoxu@sgx:~/linux/my/pro/bsp/kernel/linux/fs/ext4$ grepch __init
mballoc.c:2823:int __init ext4_init_mballoc(void)
extents_status.c:152:int __init ext4_init_es(void)
page-io.c:33:int __init ext4_init_pageio(void)
block_validity.c:30:int __init ext4_init_system_zone(void)
ext4.h:2229:extern int __init ext4_init_mballoc(void);
ext4.h:2856:extern int __init ext4_init_system_zone(void);
ext4.h:2930:extern int __init ext4_init_pageio(void);
super.c:951:static int __init init_inodecache(void)
super.c:5570:static int __init ext4_init_feat_adverts(void)
super.c:5605:static int __init ext4_init_fs(void)   // 实际上整个 ext4 初始化的入口是这个
extents_status.h:80:extern int __init ext4_init_es(void);
suguoxu@sgx:~/linux/my/pro/bsp/kernel/linux/fs/ext4$