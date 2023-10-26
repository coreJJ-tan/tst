1. int sysfs_create_dir_ns(struct kobject *kobj, const void *ns)
    创建一个目录
    kobj : 要创建的目录对应的 kobject
    ns : 该 kobject 对应的 kernfs_node 的命名空间 tag, 可为 NULL

    返回值 : 成功返回 0, 失败返回负指针

    * 该目录的 kernfs_node 将于 kobj->parent->sd 之下, 若 kobj->parent 为 NULL, 则位于 sysfs 的根下, 即 sysfs_root_kn (这也是个 kernfs_node)之下

2. void sysfs_remove_dir(struct kobject *kobj)
    删除一个目录
    kobj : 要删除的目录对应的 kobject

    会先删除所有目录下的 attribute 文件，再删除该目录的 kernfs_node 节点

3. int sysfs_rename_dir_ns(struct kobject *kobj, const char *new_name, const void *new_ns)
    重命名目录名
    kobj : 要修改的目录对应的 kobject
    new_name : 新名字
    new_ns : 该 kobject 对应的 kernfs_node 的命名空间 tag, 可为 NULL

    返回值 : 

4. int sysfs_move_dir_ns(struct kobject *kobj, struct kobject *new_parent_kobj, const void *new_ns)
    移动目录
    kobj : 要移动的目录对应的 kobject
    new_parent_kobj : 要移动到哪个新的 kobject 之下
    new_ns : 该 kobject 对应的 kernfs_node 的命名空间 tag, 可为 NULL

    返回值 : 

    * 如果 new_parent_kobj 为空，则移动到 sysfs 的根下, 即 sysfs_root_kn (这也是个 kernfs_node)之下

5. int sysfs_create_mount_point(struct kobject *parent_kobj, const char *name)

6. void sysfs_remove_mount_point(struct kobject *parent_kobj, const char *name)