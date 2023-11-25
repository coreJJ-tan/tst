1. int driver_for_each_device(struct device_driver *drv, struct device *start, void *data, int (*fn)(struct device *, void *))
    对于 drv 驱动下的设备, 从 start 设备开始, 遍历 drv 驱动下的剩余设备, 调用 fn() 函数, 有一个调用失败即返回
    data: 传给 fn() 函数的参数

2. struct device *driver_find_device(struct device_driver *drv, struct device *start, void *data, int (*match)(struct device *dev, void *data))
    对于 drv 驱动下的设备, 从 start 设备开始, 调用 match() 函数, 用于找到匹配某一规则的设备, 并返回该设备的指针, 同时其引用计数加一
    data: 传给 match() 函数的参数

3. int driver_create_file(struct device_driver *drv, const struct driver_attribute *attr)
    创建设备驱动的属性文件, 创建于 drv->p->kobj 之下, 该 kobject 对应的目录为?

4. void driver_remove_file(struct device_driver *drv, const struct driver_attribute *attr)
    删除设备驱动的属性文件

5. int driver_add_groups(struct device_driver *drv, const struct attribute_group **groups)
    创建一组设备驱动属性文件, 创建于 drv->p->kobj 之下, 该 kobject 对应的目录为?

6. void driver_remove_groups(struct device_driver *drv, const struct attribute_group **groups)
    删除一组设备驱动的属性文件

7. int driver_register(struct device_driver *drv)
    注册一个设备驱动 drv

    * 如果该驱动已经注册过, 那么会注册失败, 返回 -EBUSY. // drv->bus->p->drivers_kset 下找到名为 drv->name 的 kobject, 即为已注册
    * 调用 bus_add_driver(drv) 函数给总线添加上驱动 // 具体移步 bus.c
    * 调用 driver_add_groups(drv, drv->groups) 创建一组驱动的属性文件
    * 调用 kobject_uevent(&drv->p->kobj, KOBJ_ADD);

8. void driver_unregister(struct device_driver *drv)

9. struct device_driver *driver_find(const char *name, struct bus_type *bus)
    在总线下 bus 下找到名为 name 的驱动的实例