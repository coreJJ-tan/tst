1. 初始化部分
    int __init buses_init(void)
        bus_kset = kset_create_and_add("bus", &bus_uevent_ops, NULL); // 创建一个kset，体现为 /sys/bus 目录
        system_kset = kset_create_and_add("system", NULL, &devices_kset->kobj); // 创建一个kset，体现为 /sys/devices/system 目录
        return 0;

2. int bus_create_file(struct bus_type *bus, struct bus_attribute *attr)
    在总线 bus 所在的目录下创建一个文件 attr->attr

    * 该文件的读写函数为 attr->show() 和 attr->store()

3. void bus_remove_file(struct bus_type *bus, struct bus_attribute *attr)
    移除总线 bus 下的 attr->attr 文件

4. bind, unbind, uevent 三个只写文件
    static DRIVER_ATTR_WO(bind);    // 写函数 bind_store() 细节待定
    static DRIVER_ATTR_WO(unbind);  // 写函数 unbind_store() 细节待定
    static DRIVER_ATTR_WO(uevent);  // 写函数 uevent_store() 细节待定

5. drivers_probe, drivers_autoprobe, uevent 三个文件
    static BUS_ATTR(drivers_probe, S_IWUSR, NULL, store_drivers_probe); // 写函数 store_drivers_probe() 细节待定
    static BUS_ATTR(drivers_autoprobe, S_IWUSR | S_IRUGO, show_drivers_autoprobe, store_drivers_autoprobe); // 读函数 show_drivers_autoprobe()/写函数 store_drivers_autoprobe() 细节待定
    static BUS_ATTR(uevent, S_IWUSR, NULL, bus_uevent_store); // 写函数 bus_uevent_store() 细节待定

6. int bus_for_each_dev(struct bus_type *bus, struct device *start, void *data, int (*fn)(struct device *, void *))
    从 start 设备开始遍历总线 bus 下的所有设备, 调用 fn() 函数

    * start 如果是 NULL, 那么从第一个设备开始遍历
    * 遍历到的设备作为 fn() 的第一个形参传入, data 作为 fn() 的第二个形参
    * 如果调用 fn() 过程中有错误, 那么直接报错返回

7. struct device *bus_find_device(struct bus_type *bus, struct device *start, void *data, int (*match)(struct device *dev, void *data))
    从 start 设备开始遍历总线 bus 下的所有设备, 调用 match() 函数查找匹配某种规则的设备实例

    * start 如果是 NULL, 那么从第一个设备开始遍历
    * 遍历到的设备作为 match() 的第一个形参传入, data 作为 match() 的第二个形参
    * 如果调用 match() 返回值不为 0, 则说明找到了, 并将遍历到的设备实例作为返回值返回

8. struct device *bus_find_device_by_name(struct bus_type *bus, struct device *start, const char *name)
    从 start 设备开始遍历总线 bus 下的所有设备, 返回设备名为 name 的设备实例

    * start 如果是 NULL, 那么从第一个设备开始遍历

9. struct device *subsys_find_device_by_id(struct bus_type *subsys, unsigned int id, struct device *hint)
    便利 subsys 总线下的所有设备, 找到 device->id = id 的设备, 并返回该设备的实例

    * 如果 hint 不为 NULL, 那么优先判断该设备的 id, 如果不是该设备, 再从头开始遍历其它所有的设备进行判断

10. int bus_for_each_drv(struct bus_type *bus, struct device_driver *start, void *data, int (*fn)(struct device_driver *, void *)) // 该函数类似 bus_for_each_dev()
    从 start 设备开始遍历总线 bus 下的所有驱动, 调用 fn() 函数

    * start 如果是 NULL, 那么从第一个驱动开始遍历
    * 遍历到的驱动作为 fn() 的第一个形参传入, data 作为 fn() 的第二个形参
    * 如果调用 fn() 过程中有错误, 那么直接报错返回

11. int bus_add_device(struct device *dev)
    添加 dev 设备到设备所挂的总线 dev->bus 上

    * 在 device 所属的 sysfs 目录下创建 dev->bus 总线上默认给设备准备的属性文件 // dev->bus->dev_attrs[] 就是保存这些属性文件的数组
    * 在 device 所属的 sysfs 目录下创建 dev->bus->dev_groups 中指定的所有组的文件
    * 在该总线的设备目录下 (dev->bus->p->devices_kset->kobj) 创建软链接, 指向实际 dev 设备的目录 (dev->kobj), 软链接名就是设备名
    * 在该设备目录下 (dev->kobj) 创建软链接, 指向设备总线所属的目录 (dev->bus->p->subsys.kobj), 软链接名为 "subsystem"

12. void bus_probe_device(struct device *dev)