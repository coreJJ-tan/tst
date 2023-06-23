    sysfs提供了4类文件的创建与管理，分别是目录、普通文件、软链接文件、二进制文件

// 宏定义篇
__ATTR(_name, _mode, _show, _store)
__ATTR_PREALLOC(_name, _mode, _show, _store)
__ATTR_RO(_name)
__ATTR_WO(_name)
__ATTR_RW(_name)
__ATTR_NULL

// dir篇

// file篇
static inline int __must_check sysfs_create_file(struct kobject *kobj, const struct attribute *attr)
{
    return sysfs_create_file_ns(kobj, attr, NULL);
}
int sysfs_create_files(struct kobject *kobj, const struct attribute **ptr)  // 相当于调用多个 sysfs_create_file()，在kobj目录下创建多个属性文件
{//用于一次创建多个属性文件
    int err = 0;
    int i;
    for (i = 0; ptr[i] && !err; i++)
        err = static inline int __must_check sysfs_create_file(struct kobject *kobj, const struct attribute *attr = ptr[i]);    // 在kobj对应的目录下创建attr对应的属性文件
            return sysfs_create_file_ns(kobj, attr, const void *ns = NULL);
                BUG_ON(!kobj || !kobj->sd || !attr);
                return sysfs_add_file_mode_ns(kobj->sd, attr, false, attr->mode, ns); // int sysfs_add_file_mode_ns(struct kernfs_node *parent, const struct attribute *attr, bool is_bin, umode_t mode, const void *ns)
                                                                                      // 分配kernfs_node节点及初始化，包括关联kernfs_ops对象，将kernfs_node链接
                                                                                      // 到同级rbtree，更新哈希值及时间戳，并激活这个节点
    if (err)
        while (--i >= 0)
            sysfs_remove_file(kobj, ptr[i]);
    return err;
}
static inline void sysfs_remove_file(struct kobject *kobj, const struct attribute *attr)
{
    sysfs_remove_file_ns(kobj, attr, NULL);
}
void sysfs_remove_files(struct kobject *kobj, const struct attribute **ptr) // 相当于调用多个 sysfs_remove_file()
{ // 删除kobj下的多个属性文件
    int i;
    for (i = 0; ptr[i]; i++)
        sysfs_remove_file(kobj, ptr[i]);    // 删除 kobj 下的属性文件
}
int sysfs_add_file_to_group(struct kobject *kobj, const struct attribute *attr, const char *group) // 将属性文件添加到一个已经存在的组中
{
    struct kernfs_node *parent;
    int error;
    if(group)
        parent = kernfs_find_and_get(kobj->sd, group);
    else
        parent = kobj->sd;
        kernfs_get(parent);
    if(!parent)
        return -ENOENT;
    error = sysfs_add_file(struct kernfs_node *parent, const struct attribute *attr, bool is_bin = false)；
        return sysfs_add_file_mode_ns(parent, attr, is_bin, attr->mode, NULL);
    kernfs_put(parent);
    return error;
}
void sysfs_remove_file_from_group(struct kobject *kobj, const struct attribute *attr, const char *group) // 将属性文件attr从kobj目录下的属性集合group中删除
int sysfs_chmod_file(struct kobject *kobj, const struct attribute *attr, umode_t mode)
{ // 修改attr对应的属性文件的读写权限
    struct kernfs_node *kn;
    struct iattr newattrs;
    int rc;
    kn = kernfs_find_and_get(kobj->sd, attr->name);
    if (!kn)
        return -ENOENT;
    newattrs.ia_mode = (mode & S_IALLUGO) | (kn->mode & ~S_IALLUGO);
    newattrs.ia_valid = ATTR_MODE;
    rc = kernfs_setattr(kn, &newattrs);
    kernfs_put(kn);
    return rc;
}
void sysfs_remove_file_ns(struct kobject *kobj, const struct attribute *attr, const void *ns)
{
    struct kernfs_node *parent = kobj->sd;
    kernfs_remove_by_name_ns(parent, attr->name, ns);
}
bool sysfs_remove_file_self(struct kobject *kobj, const struct attribute *attr)
{
    struct kernfs_node *parent = kobj->sd;
    struct kernfs_node *kn;
    bool ret;
    kn = kernfs_find_and_get(parent, attr->name);
    if (WARN_ON_ONCE(!kn))
        return false;
    ret = kernfs_remove_self(kn);
    kernfs_put(kn);
    return ret;
}
int sysfs_create_bin_file(struct kobject *kobj, const struct bin_attribute *attr) // 在kobj目录下创建attr对应的二进制属性文件
void sysfs_remove_bin_file(struct kobject *kobj, const struct bin_attribute *attr) // 删除二进制属性文件

// group篇
struct attribute_group {
    const char      *name;
    umode_t         (*is_visible)(struct kobject *, struct attribute *, int);
    struct attribute    **attrs;
    struct bin_attribute    **bin_attrs;
};
int sysfs_create_group(struct kobject *kobj, const struct attribute_group *grp) // 在kobj目录下创建一个属性集合，并显示集合中的属性文件。如果文件已存在，会报错。
{
    return internal_create_group(kobj, 0, grp);
}
int sysfs_create_groups(struct kobject *kobj, const struct attribute_group **groups) //创建多个组，失败一个则都不创建
{
    int error = 0;
    int i;
    if (!groups)
        return 0;
    for (i = 0; groups[i]; i++)
        error = sysfs_create_group(kobj, groups[i]);
        if (error)
            while (--i >= 0)
                sysfs_remove_group(kobj, groups[i]);
            break;
    return error;
}
int sysfs_update_group(struct kobject *kobj, const struct attribute_group *grp) // 和sysfs_create_group()类似，但是文件已存在也不会报错
{
    return internal_create_group(kobj, 1, grp);
}
void sysfs_remove_group(struct kobject *kobj, const struct attribute_group *grp) // 在kobj目录下删除一个属性集合，并删除集合中的属性文件
void sysfs_remove_groups(struct kobject *kobj, const struct attribute_group **groups) // 相当于调用多个sysfs_remove_group()
{
    int i;
    if (!groups)
        return;
    for (i = 0; groups[i]; i++)
        sysfs_remove_group(kobj, groups[i]);
}
int sysfs_merge_group(struct kobject *kobj, const struct attribute_group *grp) // 将文件合并到group中（我的理解是该kobj下的所有属性文件都被添加到该group中）
{ //如果group不存在或该group中已存在任何文件，则此函数将返回一个错误，在这种情况下，不会创建任何新文件。
    struct kernfs_node *parent;
    int error = 0;
    struct attribute *const *attr;
    int i;
    parent = kernfs_find_and_get(kobj->sd, grp->name);
    if (!parent)
        return -ENOENT;
    for ((i = 0, attr = grp->attrs); *attr && !error; (++i, ++attr))
        error = sysfs_add_file(parent, *attr, false);
    if (error)
        while (--i >= 0)
            kernfs_remove_by_name(parent, (*--attr)->name);
    kernfs_put(parent);
    return error;
}
void sysfs_unmerge_group(struct kobject *kobj, const struct attribute_group *grp) // 从group中删除文件
{
    struct kernfs_node *parent;
    struct attribute *const *attr;
    parent = kernfs_find_and_get(kobj->sd, grp->name);
    if (parent)
        for (attr = grp->attrs; *attr; ++attr)
            kernfs_remove_by_name(parent, (*attr)->name);
        kernfs_put(parent);
}
int sysfs_add_link_to_group(struct kobject *kobj, const char *group_name, struct kobject *target, const char *link_name) // 为group添加链接
{
    struct kernfs_node *parent;
    int error = 0;
    parent = kernfs_find_and_get(kobj->sd, group_name);
    if (!parent)
        return -ENOENT;
    error = sysfs_create_link_sd(parent, target, link_name);
    kernfs_put(parent);
    return error;
}
void sysfs_remove_link_from_group(struct kobject *kobj, const char *group_name, const char *link_name) // 删除group的链接
{
    struct kernfs_node *parent;
    parent = kernfs_find_and_get(kobj->sd, group_name);
    if (parent)
        kernfs_remove_by_name(parent, link_name);
        kernfs_put(parent);
}

// link篇
int sysfs_create_link(struct kobject *kobj, struct kobject *target, const char *name) // 在kobj目录下创建指向target目录的软链接，name为软链接文件名称。
{
    return sysfs_do_create_link(kobj, target, name, 1);
}
int sysfs_create_link_nowarn(struct kobject *kobj, struct kobject *target,  const char *name) // 与sysfs_create_link()功能相同，只是在软链接文件已存在时不会出现警告。
{
    return sysfs_do_create_link(kobj, target, name, 0);
}
void sysfs_delete_link(struct kobject *kobj, struct kobject *targ, const char *name) // 与sysfs_remove_link不同，sysfs_delete_link有足够的信息来成功删除标记目录中的符号链接。
void sysfs_remove_link(struct kobject *kobj, const char *name) // 删除kobj目录下名为name的软链接文件。
int sysfs_rename_link_ns(struct kobject *kobj, struct kobject *targ, const char *old, const char *new, const void *new_ns) // 重命名targ目录中的符号链接。

// mount篇
int __init sysfs_init(void) // Sysfs文件系统的注册由sysfs_init()函数完成的

