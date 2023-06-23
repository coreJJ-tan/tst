// 宏定义篇
#define to_dev_attr(_attr) container_of(_attr, struct device_attribute, attr)
#define to_ext_attr(x) container_of(x, struct dev_ext_attribute, attr)

// 普通接口篇
void lock_device_hotplug(void)
{
    mutex_lock(&device_hotplug_lock); // device_hotplug_lock是定义在core.c中的静态全局变量，是一个互斥锁（锁什么呢？）
}
void unlock_device_hotplug(void)
{
    mutex_unlock(&device_hotplug_lock);
}
int lock_device_hotplug_sysfs(void)
{
    if(mutex_trylock(&device_hotplug_lock))
        return 0;
    msleep(5); /* Avoid busy looping (5 ms of sleep should do). */
    return restart_syscall();
}
const char *dev_driver_string(const struct device *dev) // 获取设备绑定的驱动的名字
{ // 如果没有绑定驱动，则返回绑定的bus的名字，如果也没有绑定bus，则返回绑定的class的名字，如果还没有，就返回空字符串
    struct device_driver *drv;
    drv = ACCESS_ONCE(dev->driver);
    return drv ? drv->name : (dev->bus ? dev->bus->name : (dev->class ? dev->class->name : ""));
}

// 重要接口篇
void device_initialize(struct device *dev)
{
    dev->kobj.kset = devices_kset; // devices_kset在 devices_init()函数中被创建
    kobject_init(&dev->kobj, &device_ktype); // 初始化devices这个kobject?  在哪里进行kobject_add呢？
    INIT_LIST_HEAD(&dev->dma_pools); // DMA相关，暂不了解
    mutex_init(&dev->mutex);
    lockdep_set_novalidate_class(&dev->mutex);
    spin_lock_init(&dev->devres_lock);
    INIT_LIST_HEAD(&dev->devres_head);
    device_pm_init(dev);
    set_dev_node(dev, -1);
}



//sysfs相关篇
{ // device_ktype
static ssize_t dev_attr_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
    struct device_attribute *dev_attr = to_dev_attr(attr);
    struct device *dev = kobj_to_dev(kobj);
    ssize_t ret = -EIO;
    if (dev_attr->show)
        ret = dev_attr->show(dev, dev_attr, buf);
    if (ret >= (ssize_t)PAGE_SIZE) { // 改属性文件返回值不能超过PAGE_SIZE，也就是说属性文件的"内容"所占的字节数不能超过PAGE_SIZE
        print_symbol("dev_attr_show: %s returned bad count\n", (unsigned long)dev_attr->show);
    }
    return ret;
}
static ssize_t dev_attr_store(struct kobject *kobj, struct attribute *attr,const char *buf, size_t count)
{
    struct device_attribute *dev_attr = to_dev_attr(attr);
    struct device *dev = kobj_to_dev(kobj);
    ssize_t ret = -EIO;
    if (dev_attr->store)
        ret = dev_attr->store(dev, dev_attr, buf, count);
    return ret;
}
static const struct sysfs_ops dev_sysfs_ops = {
    .show   = dev_attr_show,
    .store  = dev_attr_store,
};
static const void *device_namespace(struct kobject *kobj)
{
    struct device *dev = kobj_to_dev(kobj);
    const void *ns = NULL;
    if (dev->class && dev->class->ns_type)
        ns = dev->class->namespace(dev);
    return ns;
}
static void device_release(struct kobject *kobj)
{ // 有三种方式进行释放该kobject所占用的内存
    struct device *dev = kobj_to_dev(kobj);
    struct device_private *p = dev->p;
    /*
     * Some platform devices are driven without driver attached
     * and managed resources may have been acquired.  Make sure
     * all resources are released.
     *
     * Drivers still can add resources into device after device
     * is deleted but alive, so release devres here to avoid
     * possible memory leak.
     */
    devres_release_all(dev);
    if (dev->release)
        dev->release(dev);
    else if (dev->type && dev->type->release)
        dev->type->release(dev);
    else if (dev->class && dev->class->dev_release)
        dev->class->dev_release(dev);
    else
        WARN(1, KERN_ERR "Device '%s' does not have a release() function, it is broken and must be fixed.\n", dev_name(dev));
    kfree(p);
}
static struct kobj_type device_ktype = {
    .release    = device_release,
    .sysfs_ops  = &dev_sysfs_ops,
    .namespace  = device_namespace,
};
}
int device_add_groups(struct device *dev, const struct attribute_group **groups)
{
    return sysfs_create_groups(&dev->kobj, groups);
}
void device_remove_groups(struct device *dev, const struct attribute_group **groups)
{
    sysfs_remove_groups(&dev->kobj, groups);
}
int device_create_file(struct device *dev, const struct device_attribute *attr)
{
    int error = 0;
    if (dev) {
        WARN(((attr->attr.mode & S_IWUGO) && !attr->store), "Attribute %s: write permission without 'store'\n", attr->attr.name);
        WARN(((attr->attr.mode & S_IRUGO) && !attr->show), "Attribute %s: read permission without 'show'\n", attr->attr.name);
        error = sysfs_create_file(&dev->kobj, &attr->attr);
    }
    return error;
}
void device_remove_file(struct device *dev, const struct device_attribute *attr)
{
    if (dev)
        sysfs_remove_file(&dev->kobj, &attr->attr);
}
static DEVICE_ATTR_RW(uevent); // 这个宏展开相当于下面的代码， device_add()函数会使用它 ---> dev_attr_uevent
{ //定义一个 dev_attr_uevent 的设备属性，设定对应的show和store函数
    struct device_attribute dev_attr_uevent = {
    .attr = {.name = __stringify(uevent),
         .mode = VERIFY_OCTAL_PERMISSIONS(S_IWUSR | S_IRUGO) },
    .show   = uevent_show,
    .store  = uevent_store,
    }
}
static DEVICE_ATTR_RW(online); // 定义 dev_attr_online 设备属性，解释同上, device_add()函数会调用device_add_attrs()使用它 ---> dev_attr_online
int device_create_bin_file(struct device *dev, const struct bin_attribute *attr)
{
    int error = -EINVAL;
    if (dev)
        error = sysfs_create_bin_file(&dev->kobj, attr);
    return error;
}
void device_remove_bin_file(struct device *dev, const struct bin_attribute *attr)
{
    if (dev)
        sysfs_remove_bin_file(&dev->kobj, attr);
}

// 随系统初始化篇
int __init devices_init(void)
{
    devices_kset = kset_create_and_add("devices", &device_uevent_ops, NULL); // 创建一个kset，体现为/sys/devices目录
    if (!devices_kset)
        return -ENOMEM;
    dev_kobj = kobject_create_and_add("dev", NULL); // 创建一个kobject，体现为/sys/dev目录
    if (!dev_kobj)
        goto dev_kobj_err;
    sysfs_dev_block_kobj = kobject_create_and_add("block", dev_kobj); // 创建一个kobject，体现为/sys/dev/block目录
    if (!sysfs_dev_block_kobj)
        goto block_kobj_err;
    sysfs_dev_char_kobj = kobject_create_and_add("char", dev_kobj); // 创建一个kobject，体现为/sys/dev/char目录
    if (!sysfs_dev_char_kobj)
        goto char_kobj_err;

    return 0;

 char_kobj_err:
    kobject_put(sysfs_dev_block_kobj);
 block_kobj_err:
    kobject_put(dev_kobj);
 dev_kobj_err:
    kset_unregister(devices_kset);
    return -ENOMEM;
}

看到654行 ，完善一下device_initialize函数