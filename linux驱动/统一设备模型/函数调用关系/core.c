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
    dev->kobj.kset = devices_kset;
    kobject_init(&dev->kobj, &device_ktype);
    INIT_LIST_HEAD(&dev->dma_pools);
    mutex_init(&dev->mutex);
    lockdep_set_novalidate_class(&dev->mutex);
    spin_lock_init(&dev->devres_lock);
    INIT_LIST_HEAD(&dev->devres_head);
    device_pm_init(dev);
    set_dev_node(dev, -1);
}

//sysfs相关篇
{
static ssize_t dev_attr_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
    struct device_attribute *dev_attr = to_dev_attr(attr);
    struct device *dev = kobj_to_dev(kobj);
    ssize_t ret = -EIO;
    if (dev_attr->show)
        ret = dev_attr->show(dev, dev_attr, buf);
    if (ret >= (ssize_t)PAGE_SIZE) {
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
{
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
        WARN(1, KERN_ERR "Device '%s' does not have a release() "
            "function, it is broken and must be fixed.\n",
            dev_name(dev));
    kfree(p);
}
static struct kobj_type device_ktype = {
    .release    = device_release,
    .sysfs_ops  = &dev_sysfs_ops,
    .namespace  = device_namespace,
};
}

看到147行