void sysfs_notify(struct kobject *kobj, const char *dir, const char *attr) // 可以理解为一种通信机制，用来唤醒在读写属性文件(sysfs节点)时因调用select()或poll()而阻塞的用户进程。

1. int sysfs_add_file_mode_ns(struct kernfs_node *parent, const struct attribute *attr, bool is_bin, umode_t mode, const void *ns)
    该文件用于创建一个文件。
    parent : 该文件创建于哪个 kernfs_node 之下
    attr : 代表该文件的 attribute 实例
    is_bin : 非 0 表示创建二进制文件
    mode : 该文件访问权限, 会根据该模式选择不同的文件操作函数集实例, 具体的函数集详见 《0.2-创建文件的类型.c》
    ns : kernfs_node 的命名空间 tag, 可为 NULL

    返回值 : 成功返回 0, 失败返回负指针

2. int sysfs_add_file(struct kernfs_node *parent, const struct attribute *attr, bool is_bin)
        return sysfs_add_file_mode_ns(parent, attr, is_bin, attr->mode, NULL);
    用于创建一个文件
    parent : 该文件创建于哪个 kernfs_node 之下
    attr : 代表该文件的 attribute 实例，文件的模式由 attr->mode 指定
    is_bin : 非 0 表示创建二进制文件

    返回值 : 成功返回 0, 失败返回负指针

3. int sysfs_create_file_ns(struct kobject *kobj, const struct attribute *attr, const void *ns)
    用于创建一个文本文件。
    kobj : 该文件创建于哪个 kobject 之下
    attr : 代表该文件的 attribute 实例，文件的模式由 attr->mode 指定
    ns : kernfs_node 的命名空间 tag, 可为 NULL

    返回值 : 成功返回 0, 失败返回负指针

4. static inline int __must_check sysfs_create_file(struct kobject *kobj, const struct attribute *attr)
        return sysfs_create_file_ns(kobj, attr, NULL);
    用于创建一个文本文件。
    kobj : 该文件创建于哪个 kobject 之下
    attr : 代表该文件的 attribute 实例，文件的模式由 attr->mode 指定

    返回值 : 成功返回 0, 失败返回负指针

5. int sysfs_create_files(struct kobject *kobj, const struct attribute **ptr)
        for (i = 0; ptr[i] && !err; i++)  // 这种方式就需要保证 ptr 的最后一个成员必须指向 NULL
            sysfs_create_file(kobj, ptr[i]);
    用于创建一个一系列文本文件。
    kobj : 这些文件创建于哪个 kobject 之下
    ptr : 指向一个二维数组，每个成员指向要创建的文件的 attribute 实例

    返回值 : 成功返回 0, 失败返回负指针

6. int sysfs_add_file_to_group(struct kobject *kobj, const struct attribute *attr, const char *group)
    创建一个文件并添加到一个已经存在的组中。
    kobj: 该文件创建于哪个 kobject 之下
    attr: 代表该文件的 attribute 实例
    group: 相应的 group 的名字

    返回值 : 成功返回 0, 失败返回负指针

7. int sysfs_chmod_file(struct kobject *kobj, const struct attribute *attr, umode_t mode)
    修改文件的访问权限
    kobj: 该文件位于哪个 kobject 之下
    attr: 代表该文件的 attribute 实例
    mode : 该文件新的访问权限

    返回值 : 成功返回 0, 失败返回负指针 (e.g 文件不存在返回 return -ENOENT)

8. void sysfs_remove_file_ns(struct kobject *kobj, const struct attribute *attr, const void *ns)
    根据文件的名字和命名空间来删除一个文件, 即sysfs_create_file_ns() 函数的反操作
    kobj: 该文件位于哪个 kobject 之下
    attr: 代表该文件的 attribute 实例，会根据 attr->name 来删除文件
    ns : kernfs_node 的命名空间 tag, 可为 NULL

9. bool sysfs_remove_file_self(struct kobject *kobj, const struct attribute *attr)
    删除一个文件, 和 sysfs_remove_file_ns() 不一样，它不是根据文件名和命名空间删除的，它先获取代表文件的 kernfs_node, 并根据此删除
    kobj: 该文件位于哪个 kobject 之下
    attr: 代表该文件的 attribute 实例

    返回值：

10. void sysfs_remove_files(struct kobject *kobj, const struct attribute **ptr)
        for (i = 0; ptr[i]; i++)  // 这种方式就需要保证 ptr 的最后一个成员必须指向 NULL
		    sysfs_remove_file(kobj, ptr[i]);
    删除一系列的文件
    kobj : 这些文件创建于哪个 kobject 之下
    ptr : 指向一个二维数组，每个成员指向要创建的文件的 attribute 实例

11. void sysfs_remove_file_from_group(struct kobject *kobj, const struct attribute *attr, const char *group)
    将一个文件从一个组中删除并从sysfs下删除
    kobj: 该文件创建于哪个 kobject 之下
    attr: 代表该文件的 attribute 实例，会根据 attr->name 来删除文件
    group: 相应的 group 的名字+

12. int sysfs_create_bin_file(struct kobject *kobj, const struct bin_attribute *attr)
    创建一个二进制文件
    kobj: 该文件创建于哪个 kobject 之下
    attr: 代表该文件的 bin_attribute 实例, 会根据 bin_attribute->attr 调用 sysfs_add_file() 来创建

    注意：此事要先实现好 bin_attribute 的 ->read()、->write()、->mmap() 三个函数

    返回值 : 成功返回 0, 失败返回负指针

13. void sysfs_remove_bin_file(struct kobject *kobj, const struct bin_attribute *attr)
    删除一个二进制文件
    kobj: 该文件创建于哪个 kobject 之下
    attr: 代表该文件的 bin_attribute 实例