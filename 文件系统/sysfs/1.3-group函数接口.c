1. int sysfs_create_group(struct kobject *kobj, const struct attribute_group *grp)
    创建一个组，并创建这个组中的所有文件，包括其中的文本文件和二进制文件
    kobj : 要创建的组对应的 kobject
    grp : 要创建的组

    返回值 : 成功返回 0, 失败返回负指针

    * 如果这个组名 grp->name 被事先指定了，那么将在 kobj 对应的 kernfs_node 下创建一个新的目录，目录名就是组名，随后在该目录下创建组中指定的所有文件，需要注意的是，这
个目录并不代表一个 kobject.
    * grp->attrs 和 grp->bin_attrs 必须至少存在其一，否则创建失败
    * 在创建一个文件失败时，将会移除所有已创建过的组中的文件。
    * 组中的文件读写接口是怎么实现的？

2. int sysfs_create_groups(struct kobject *kobj, const struct attribute_group **groups)
    调用 sysfs_create_group() 函数创建多个组, 有一个组创建失败，则移除创建过的所有组

    * groups 成员的最后一个指针必须指向 NULL ?

3. void sysfs_remove_group(struct kobject *kobj, const struct attribute_group *grp)

4. void sysfs_remove_groups(struct kobject *kobj, const struct attribute_group **groups)