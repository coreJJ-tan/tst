1. 初始化部分
    int __init buses_init(void)
        bus_kset = kset_create_and_add("bus", &bus_uevent_ops, NULL); // 创建一个kset, 体现为 /sys/bus 目录
        system_kset = kset_create_and_add("system", NULL, &devices_kset->kobj); // 创建一个kset, 体现为 /sys/devices/system 目录
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
        该文件为 1 表示允许总线下的设备在注册时自动匹配驱动
    static BUS_ATTR(uevent, S_IWUSR, NULL, bus_uevent_store); // 写函数 bus_uevent_store() 细节待定

6. int bus_for_each_dev(struct bus_type *bus, struct device *start, void *data, int (*fn)(struct device *, void *))
    从 start 设备开始遍历总线 bus 下的所有设备, 调用 fn() 函数

    * start 如果是 NULL, 那么从第一个设备开始遍历
    * 遍历到的设备作为 fn() 的第一个形参传入, data 作为 fn() 的第二个形参
    * 如果调用 fn() 过程中有错误, 那么直接报错返回 // fn() 返回 0 表示无错误

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
    * 如果调用 fn() 过程中有错误, 那么直接报错返回 // fn() 返回 0 表示无错误

11. int bus_add_device(struct device *dev)
    添加 dev 设备到设备所挂的总线 dev->bus 上

    * 在 device 所属的 sysfs 目录下创建 dev->bus 总线上默认给设备准备的属性文件 // dev->bus->dev_attrs[] 就是保存这些属性文件的数组
    * 在 device 所属的 sysfs 目录下创建 dev->bus->dev_groups 中指定的所有组的文件
    * 在该总线的设备目录下 (dev->bus->p->devices_kset->kobj) 创建软链接, 指向实际 dev 设备的目录 (dev->kobj), 软链接名就是设备名
    * 在该设备目录下 (dev->kobj) 创建软链接, 指向设备总线所属的目录 (dev->bus->p->subsys.kobj), 软链接名为 "subsystem"
    * 将该设备挂到设备所属的总线上, 具体是 dev->p->knode_bus 挂在 bus->p->klist_devices 之上

12. void bus_probe_device(struct device *dev)
    给设备注册驱动
    
    * 寻找总线下的驱动, 然后在设备和驱动各自的目录下创建指向对方的软链接, 并将设备的 klist 节点挂到驱动的 klist 头部上去
    // 其它细节待定

13. void bus_remove_device(struct device *dev)
    从总线下移除设备

    * 移除 sysfs 中设备和总线的目录下指向对方的软链接
    * 移除设备目录下和总线相关的属性文件
    * 将设备从总线的 klist 中移除
    * 将设备和驱动解除绑定, 互相移除指向对方的软链接, 驱动 klist 链表中移除设备等操作
    // 其它细节待定, 其实就是注册驱动时的各种反向操作

14. int bus_add_driver(struct device_driver *drv)
    添加 drv 驱动到驱动所属的总线 drv->bus 上

    * 初始化驱动的设备 klist 链表 (drv->p->klist_devices), 该链表将链接一连串的设备
    * 在 /sys/bus/xxx/drivers 下创建驱动的目录 "drivers"
    * 将该驱动添加到总线的驱动 klist 链表 (bus->p->klist_drivers) 上
    * 如果允许本条总线上的 device 注册时自动匹配 driver (即 drv->bus->p->drivers_autoprobe 非 0), 那么遍历总线上的设备, 如果有设备和本驱动匹配上了, 那么就给该设备注册上本驱动
    * 在本驱动的目录下创建 uevent 文件
    * 在本驱动的目录下创建驱动所属的总线的共有文件 (bus->drv_groups 指定)
    * 如果本驱动支持在用户空间手动进行设备和驱动的绑定 (即 drv->suppress_bind_attrs = 0), 那么在该驱动目录下创建 bind 和 unbing 文件, 用于手动绑定和解绑定设备

15. void bus_remove_driver(struct device_driver *drv)
    bus_add_driver() 函数的反操作

16. int bus_rescan_devices(struct bus_type *bus)
    遍历总线上的所有设备, 给没有注册过驱动的设备注册驱动

17. int device_reprobe(struct device *dev)
    将设备原先注册的驱动注销后, 再启动新驱动的注册过程
    
    * 这一般用于在设备使用的过程中想要更换驱动时, 调用该函数之前驱动和设备匹配的规则应该做相应的变化

18. int bus_register(struct bus_type *bus)
    向内核添加总线 bus // 有个疑问, 该总线添加后, 内核通过什么方式找到该总线的指针呢？

    * 在 /sys/bus 下创建属于总线的目录, 该目录的 kobject 也是一个 kset
    * 在上面刚创建好的总线目录下创建 "uevent" 文件
    * 在上面刚创建好的总线目录下创建 "devices" 目录
    * 在上面刚创建好的总线目录下创建 "drivers" 目录
    * 初始化总线下的设备 klist 链表, 该链表将链接一连串的设备
    * 初始化总线下的驱动 klist 链表, 该链表将链接一连串的驱动
    * 在总线目录下创建 "drivers_probe" 和 "drivers_autoprobe" 两个属性文件
    * 在总线目录下创建 bus->bus_groups 指定的文件

19. void bus_unregister(struct bus_type *bus)
    注销总线

    * 移除总线目录下的 bus->bus_groups 指定的文件
    * 移除总线目录下的 "drivers_probe" 和 "drivers_autoprobe" 两个属性文件
    * 移除总线目录下的 "drivers" 目录, 这是一个 kset
    * 移除总线目录下的 "devices" 目录, 这是一个 kset
    * 移除总线目录下的 "uevent" 文件
    * 移除总线目录, 这是一个 kset 

20. int bus_register_notifier(struct bus_type *bus, struct notifier_block *nb)
    注册总线 bus 的内核通知链

    * 当有事件在该总线上发生时, 可以通知到总线上所有注册通知链的地方, 执行 nb->notifier_call() 回调函数

21. int bus_unregister_notifier(struct bus_type *bus, struct notifier_block *nb)
    注销一个总线 bus 上的通知链节点

22. struct kset *bus_get_kset(struct bus_type *bus)
        return &bus->p->subsys;

23. struct klist *bus_get_device_klist(struct bus_type *bus)
        return &bus->p->klist_devices; // bus->p->klist_devices 是链接一连串设备的 klist 链表头

24. void bus_sort_breadthfirst(struct bus_type *bus, int (*compare)(const struct device *a, const struct device *b))
    对总线 bus 下的设备链表进行重新排序, 排序的规则按照 compare() 的调用结果给出

    * 遍历原链表, 对于相邻的两个节点, 会带入compare()作为形参, 返回值小于等于0, 那么 a 设备的节点将位于 b 设备的节点之后
    * 时间复杂度 O(n^2), 内部借助一个临时链表, 先将一个个原链表的节点取出并删除在源节点, 按照compare()的返回值插到临时链表中, 最后再将临时链表拼接到原链表之后

25. void subsys_dev_iter_init(struct subsys_dev_iter *iter, struct bus_type *subsys, struct device *start, const struct device_type *type)
26. struct device *subsys_dev_iter_next(struct subsys_dev_iter *iter)
27. void subsys_dev_iter_exit(struct subsys_dev_iter *iter)
    这三个函数组合起来可以当做一个迭代器使用, 这个迭代器的迭代对象是总线上的设备.

    * subsys_dev_iter_init() 用于初始化这个迭代器, iter 就是要初始化的迭代器, 只需要事先定义好即可, subsys 指定要迭代的总线, start 指定要从总线上哪个设备开始迭代, 如果指定为 NULL, 
那么会从头开始迭代, type 表示指定每次迭代到的设备的设备类型.
    * subsys_dev_iter_next() 用于进行每一次迭代, 调用一次即完成了一次迭代, 函数内部会在总线的设备链表上查找设备, 如果设备的设备类型 device->type 不是初始化时指定的类型, 那么继续
迭代到下一个设备, 如果初始化时设备类型未指定(即 NULL), 那么每个设备都会被迭代到.
    * subsys_dev_iter_exit() 用于退出这个迭代器, 主要是释放目前迭代到的设备的 klist 节点的引用计数.

28. int subsys_interface_register(struct subsys_interface *sif)
    向总线 sif->subsys 注册一个 subsys_interface

    * 将该 subsys_interface 挂到所属总线的 sif->subsys->p->interfaces 链表上
    * 如果该 subsys_interface 定义了 sif->add_dev() 函数, 那么循环获取总线上的设备，调用 subsys_interface->add_dev() 函数

29. void subsys_interface_unregister(struct subsys_interface *sif)
    将一个 subsys_interface 从总线 sif->subsys 上注销

    * 将该 subsys_interface 从总线的 sif->subsys->p->interfaces 链表上移除
    * 如果该 subsys_interface 定义了 sif->remove_dev() 函数, 那么循环获取总线上的设备，调用 subsys_interface->remove_dev() 函数

30. int subsys_system_register(struct bus_type *subsys, const struct attribute_group **groups)
    注册总线 subsys, 并将该总线看成一个设备进行注册

    * 调用 bus_register() 函数注册总线 subsys
    * 调用 device_register() 将总线注册成设备, 对应的 sysfs 目录为 /sys/devices/system/"总线名"

31. int subsys_virtual_register(struct bus_type *subsys, const struct attribute_group **groups)
    注册总线 subsys, 并将该总线看成一个设备进行注册

    * 调用 bus_register() 函数注册总线 subsys
    * 调用 device_register() 将总线注册成设备, 对应的 sysfs 目录为 /sys/devices/virtual/"总线名"