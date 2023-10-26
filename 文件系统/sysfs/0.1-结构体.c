struct sysfs_ops {
	ssize_t	(*show)(struct kobject *, struct attribute *, char *);
	ssize_t	(*store)(struct kobject *, struct attribute *, const char *, size_t);
};

struct attribute {
    const char      *name;  // name表示文件名称
    umode_t         mode;   // mode表示文件的操作权限
    #ifdef CONFIG_DEBUG_LOCK_ALLOC
        bool            ignore_lockdep:1;
        struct lock_class_key   *key;
        struct lock_class_key   skey;
    #endif
};
struct bin_attribute {
    struct attribute    attr;
    size_t          size;
    void            *private;
    ssize_t (*read)(struct file *, struct kobject *, struct bin_attribute *, char *, loff_t, size_t); // 二进制文件的读函数
    ssize_t (*write)(struct file *, struct kobject *, struct bin_attribute *, char *, loff_t, size_t); // 二进制文件的写函数
    int (*mmap)(struct file *, struct kobject *, struct bin_attribute *attr, struct vm_area_struct *vma);
};

struct attribute_group {
    const char      *name;
    umode_t         (*is_visible)(struct kobject *, struct attribute *, int);
    struct attribute    **attrs; // 这是个二维指针数组，attrs 的最后一个指针必须指向 NULL
    struct bin_attribute    **bin_attrs; // bin_attrs 的最后一个指针必须指向 NULL
};