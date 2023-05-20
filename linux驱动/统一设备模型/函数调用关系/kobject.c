// kobject篇
void kobject_init(struct kobject *kobj, struct kobj_type *ktype)
{
	char *err_str;
	if (!kobj)
		goto error; //err_str = "invalid kobject pointer!";
	if (!ktype)
		goto error;//err_str = "must have a ktype to be initialized properly!\n";
	if (kobj->state_initialized)
		/* do not error out as sometimes we can recover */
		dump_stack(); //printk(KERN_ERR "kobject (%p): tried to init an initialized object, something is seriously wrong.\n", kobj);
	kobject_init_internal(kobj);
		if (!kobj)
			return;
		kref_init(&kobj->kref); // 初始化kobject引用计数为1
		INIT_LIST_HEAD(&kobj->entry);
		kobj->state_in_sysfs = 0;
		kobj->state_add_uevent_sent = 0;
		kobj->state_remove_uevent_sent = 0;
		kobj->state_initialized = 1;
	kobj->ktype = ktype;
	return;
	error:
		dump_stack(); //printk(KERN_ERR "kobject (%p): %s\n", kobj, err_str);
}
int kobject_add(struct kobject *kobj, struct kobject *parent, const char *fmt, ...)
{
	va_list args;
	if (!kobj)
		return -EINVAL;
	if (!kobj->state_initialized)	// 如果还没有初始化kobject，则不允许添加
		dump_stack(); //printk(KERN_ERR "kobject '%s' (%p): tried to add an uninitialized object, something is seriously wrong.\n", kobject_name(kobj), kobj);
		return -EINVAL;
	va_start(args, fmt);
	retval = int kobject_add_varg(struct kobject *kobj, struct kobject *parent, const char *fmt, va_list vargs);
		retval = kobject_set_name_vargs(kobj, fmt, vargs);//设置kobject的名字，会检查名字的合法性（符合linux中文件的命名规范）
		{
			const char *old_name = kobj->name;
			char *s;
			if (kobj->name && !fmt)
				return 0;
			kobj->name = kvasprintf(GFP_KERNEL, fmt, vargs);
			if (!kobj->name)
				kobj->name = old_name;
				return -ENOMEM;
			/* ewww... some of these buggers have '/' in the name ... */
			while ((s = strchr(kobj->name, '/')))
				s[0] = '!';
			kfree(old_name);
			return 0;
		}
		if (retval)
			return retval;//printk(KERN_ERR "kobject: can not set name properly!\n");
		kobj->parent = parent;
		return kobject_add_internal(kobj);
			int error = 0;
			struct kobject *parent;
			if (!kobj)
				return -ENOENT;
			if (!kobj->name || !kobj->name[0]) // 不允许设置空名字
				return -EINVAL; // WARN(1, "kobject: (%p): attempted to be registered with empty name!\n", kobj);
			parent = kobject_get(kobj->parent); // 设置了父亲就要增加父亲的引用计数
			if (kobj->kset)
				if (!parent)
					parent = kobject_get(&kobj->kset->kobj); // 若形参传入的父亲为空，则设置kobject的父亲为自己kset的kobject（自己kset的kobject也有可能为空）
				kobj_kset_join(kobj); // 将kobject添加到自身kset的链表中
				{
					if (!kobj->kset)
						return;
					kset_get(kobj->kset);
					spin_lock(&kobj->kset->list_lock);
					list_add_tail(&kobj->entry, &kobj->kset->list);
					spin_unlock(&kobj->kset->list_lock);
				}
				kobj->parent = parent;
			pr_debug("kobject: '%s' (%p): %s: parent: '%s', set: '%s'\n", kobject_name(kobj), kobj, __func__, parent ? kobject_name(parent) : "<NULL>", kobj->kset ? kobject_name(&kobj->kset->kobj) : "<NULL>");
			error = create_dir(kobj); //在sysfs中创建目录
			if (error)
				kobj_kset_leave(kobj);
				kobject_put(parent);
				kobj->parent = NULL;
				if (error == -EEXIST)
					WARN(1, "%s failed for %s with -EEXIST, don't try to register things with the same name in the same directory.\n", __func__, kobject_name(kobj));
				else
					WARN(1, "%s failed for %s (error: %d parent: %s)\n", __func__, kobject_name(kobj), error, parent ? kobject_name(parent) : "'none'");
			else
				kobj->state_in_sysfs = 1; //表明该kobject已经在sysfs中成功创建了目录
			return error;
	va_end(args);
	return retval;
}
struct kobject *kobject_get(struct kobject *kobj)
{
    if (kobj)
        if (!kobj->state_initialized)
            WARN(1, KERN_WARNING "kobject: '%s' (%p): is not initialized, yet kobject_get() is being called.\n", kobject_name(kobj), kobj);
        kref_get(&kobj->kref);
    return kobj;
}
void kobject_put(struct kobject *kobj)
{
    if (kobj)
        if (!kobj->state_initialized)
            WARN(1, KERN_WARNING "kobject: '%s' (%p): is not initialized, yet kobject_put() is being called.\n", kobject_name(kobj), kobj);
        kref_put(&kobj->kref, kobject_release);
}
int kobject_init_and_add(struct kobject *kobj, struct kobj_type *ktype, struct kobject *parent, const char *fmt, ...)
{
    va_list args;
    int retval;
    kobject_init(kobj, ktype);
    va_start(args, fmt);
    retval = kobject_add_varg(kobj, parent, fmt, args);
    va_end(args);
    return retval;
}
int kobject_set_name(struct kobject *kobj, const char *fmt, ...)
{
    va_list vargs;
    int retval;
    va_start(vargs, fmt);
    retval = kobject_set_name_vargs(kobj, fmt, vargs);
    va_end(vargs);
    return retval;
}
int kobject_rename(struct kobject *kobj, const char *new_name)
int kobject_move(struct kobject *kobj, struct kobject *new_parent)
void kobject_del(struct kobject *kobj)
char *kobject_get_path(struct kobject *kobj, gfp_t gfp_mask)
{
    char *path;
    int len;
    len = get_kobj_path_length(kobj);
    if (len == 0)
        return NULL;
    path = kzalloc(len, gfp_mask);
    if (!path)
        return NULL;
    fill_kobj_path(kobj, path, len);
    return path;
}
struct kobject *kobject_create(void)
{
    struct kobject *kobj;
    kobj = kzalloc(sizeof(*kobj), GFP_KERNEL);
    if (!kobj)
        return NULL;
    kobject_init(kobj, &dynamic_kobj_ktype);
    return kobj;
}
struct kobject *kobject_create_and_add(const char *name, struct kobject *parent)
{
    struct kobject *kobj;
    int retval;
    kobj = kobject_create();
    if (!kobj)
        return NULL;
    retval = kobject_add(kobj, parent, "%s", name);
    if (retval)
        kobject_put(kobj); // printk(KERN_WARNING "%s: kobject_add error: %d\n", __func__, retval);
        kobj = NULL;
    return kobj;
}
const void *kobject_namespace(struct kobject *kobj)
{
    const struct kobj_ns_type_operations *ns_ops = kobj_ns_ops(kobj);
    if (!ns_ops || ns_ops->type == KOBJ_NS_TYPE_NONE)
        return NULL;
    return kobj->ktype->namespace(kobj);
}

//kset篇
void kset_init(struct kset *k)
{
    kobject_init_internal(&k->kobj);
    INIT_LIST_HEAD(&k->list);
    spin_lock_init(&k->list_lock);
}
int kset_register(struct kset *k)
{
    if (!k)
        return -EINVAL;
    kset_init(k);
    err = kobject_add_internal(&k->kobj);
    if (err)
        return err;
    kobject_uevent(&k->kobj, KOBJ_ADD);
    return 0;
}
void kset_unregister(struct kset *k)
{
    if (!k)
        return;
    kobject_del(&k->kobj);
    kobject_put(&k->kobj);
}
struct kobject *kset_find_obj(struct kset *kset, const char *name)
{
    struct kobject *k;
    struct kobject *ret = NULL;
    spin_lock(&kset->list_lock);
    list_for_each_entry(k, &kset->list, entry)
        if (kobject_name(k) && !strcmp(kobject_name(k), name))
            ret = kobject_get_unless_zero(k);
            break;
    spin_unlock(&kset->list_lock);
    return ret;
}
struct kset *kset_create_and_add(const char *name, const struct kset_uevent_ops *uevent_ops, struct kobject *parent_kobj)
{
    struct kset *kset;
    int error;
    kset =  static struct kset *kset_create(const char *name, const struct kset_uevent_ops *uevent_ops, struct kobject *parent_kobj)
	{
		struct kset *kset;
		int retval;
		kset = kzalloc(sizeof(*kset), GFP_KERNEL);
		if (!kset)
			return NULL;
		retval = kobject_set_name(&kset->kobj, "%s", name);
		if (retval)
			kfree(kset);
			return NULL;
		kset->uevent_ops = uevent_ops;
		kset->kobj.parent = parent_kobj;
		/*
		* The kobject of this kset will have a type of kset_ktype and belong to
		* no kset itself.  That way we can properly free it when it is
		* finished being used.
		*/
		kset->kobj.ktype = &kset_ktype;
		kset->kobj.kset = NULL;
		return kset;
	}
    if (!kset)
        return NULL;
    error = kset_register(kset);
    if (error)
        kfree(kset);
        return NULL;
    return kset;
}

{//dynamic_kobj_ktype结构相关
static struct kobj_type dynamic_kobj_ktype = {
    .release    = dynamic_kobj_release, //释放该kobject
    .sysfs_ops  = &kobj_sysfs_ops,
};
const struct sysfs_ops kobj_sysfs_ops = {
    .show   = kobj_attr_show, // 属性文件的读写会先走进这两个show和store函数
    .store  = kobj_attr_store,
};
static ssize_t kobj_attr_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
    struct kobj_attribute *kattr;
    ssize_t ret = -EIO;
    kattr = container_of(attr, struct kobj_attribute, attr);
    if (kattr->show)
        ret = kattr->show(kobj, kattr, buf); // 找到属性文件自己的读函数并执行
    return ret;
}
static ssize_t kobj_attr_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
    struct kobj_attribute *kattr;
    ssize_t ret = -EIO;
    kattr = container_of(attr, struct kobj_attribute, attr);
    if (kattr->store)
        ret = kattr->store(kobj, kattr, buf, count); // 找到属性文件自己的写函数并执行
    return ret;
}
}

{ //kset_ktype结构体相关
static struct kobj_type kset_ktype = {
    .sysfs_ops  = &kobj_sysfs_ops, // 见上
    .release = kset_release,	//释放该kset内存
};
}

