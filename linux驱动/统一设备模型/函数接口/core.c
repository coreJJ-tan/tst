0. 初始化阶段
    int __init devices_init(void)
        创建一个 kset, 名为 devices_kset, 对应 sysfs 下的 "/sys/devices" 目录
        创建一个 kobject, 名为 dev_kobj, 对应 sysfs 下的 "/sys/dev" 目录
        创建一个 kobject, 名为 sysfs_dev_block_kobj, 对应 sysfs 下的 "/sys/dev/block" 目录
        创建一个 kobject, 名为 sysfs_dev_char_kobj, 对应 sysfs 下的 "/sys/dev/char" 目录

1. const char *dev_driver_string(const struct device *dev)
    获取设备绑定的驱动的名字
    dev: 要获取的设备结构体

    返回值: 设备的驱动名 或者 绑定的总线名 或者 设备的 class 名 或者 ""

    * 如果没有绑定驱动, 则返回绑定的bus的名字, 如果也没有绑定bus, 则返回绑定的class的名字, 如果还没有, 就返回空字符串

2. #define to_dev_attr(_attr) container_of(_attr, struct device_attribute, attr)
    由输入的 struct attribute 类型的指针获取对应的 struct device_attribute 的指针
    _attr: struct attribute 类型的指针

    返回值: struct device_attribute 的指针

3. #define to_ext_attr(x) container_of(x, struct dev_ext_attribute, attr)
    由输入的 struct device_attribute 类型的指针获取对应的 struct dev_ext_attribute 的指针
    x: struct device_attribute 类型的指针

    返回值: struct dev_ext_attribute 的指针

4. ssize_t device_store_ulong(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
    将字符串转换成 unsigned long 型的数据后保存起来.
    dev: 未使用该参数
    attr: 通过 attr 获取 dev_ext_attribute 类型的指针, 随后会将转换后的数据赋值给 dev_ext_attribute->var 指向的内存
    buf: 要转换的字符串首地址
    size: buf 所占的字节数, 函数内部实际只作为返回值使用

    返回值: -EINVAL表示转换失败, 否则返回 size, 告诉调用者全部转换成功 (即使没有转换完所有的内存)

    * dev_ext_attribute 指向的内存需要事先申请好, dev_ext_attribute->var指向的内存也要申请好, 才能将转换后的数据赋值给这片内存
    * 例子: "0x22" 转换后的值为 0x22
            "0x22afkas" 转换后的值为 0x22af
            "22" 转换后的值为 22
            "022" 转换后的值为 022 (8 进制)
            "d256" 转换失败, 返回值为 -EINVAL

5. ssize_t device_show_ulong(struct device *dev, struct device_attribute *attr, char *buf)
    将保存在 dev_ext_attribute->var 中的 unsigned long 型的数据转成字符串
    dev: 未使用该参数
    attr: 通过 attr 获取 dev_ext_attribute 类型的指针, 随后获取 dev_ext_attribute->var 中的 unsigned long 型的数据
    buf: 转换后的字符串赋值给该内存

    返回值: 返回 buf 获取的字符串长度

6. ssize_t device_store_int(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
    将字符串转换成 int  型的数据后保存起来.
    dev: 未使用该参数
    attr: 通过 attr 获取 dev_ext_attribute 类型的指针, 随后会将转换后的数据赋值给 dev_ext_attribute->var 指向的内存
    buf: 要转换的字符串首地址
    size: buf 所占的字节数, 函数内部实际只作为返回值使用

    返回值: -EINVAL表示转换失败, 否则返回 size, 告诉调用者全部转换成功 (即使没有转换完所有的内存)

    * dev_ext_attribute 指向的内存需要事先申请好, dev_ext_attribute->var指向的内存也要申请好, 才能将转换后的数据赋值给这片内存
    * 如果是 64 bits 的 CPU, 只能保存高 32 bits 还是低 32 bits 的?
    * 例子: "0x22" 转换后的值为 0x22
            "0x22afkas" 转换后的值为 0x22af
            "22" 转换后的值为 22
            "022" 转换后的值为 022 (8 进制)
            "d256" 转换失败, 返回值为 -EINVAL

7. ssize_t device_show_int(struct device *dev, struct device_attribute *attr, char *buf)
    将保存在 dev_ext_attribute->var 中的 int 型的数据转成字符串
    dev: 未使用该参数
    attr: 通过 attr 获取 dev_ext_attribute 类型的指针, 随后获取 dev_ext_attribute->var 中的 int 型的数据
    buf: 转换后的字符串赋值给该内存

    返回值: 返回 buf 获取的字符串长度

8. ssize_t device_store_bool(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)

9. ssize_t device_show_bool(struct device *dev, struct device_attribute *attr, char *buf)

10. int device_add_groups(struct device *dev, const struct attribute_group **groups)
    return sysfs_create_groups(&dev->kobj, groups);
    调用 sysfs_create_groups() 在 device->kobj 的目录下创建 groups 里面指定的所有文件

11. void device_remove_groups(struct device *dev, const struct attribute_group **groups)
    sysfs_remove_groups(&dev->kobj, groups);
    调用 sysfs_remove_groups() 移除 device->kobj 的目录下 groups 里面指定的所有文件

12. int device_create_file(struct device *dev, const struct device_attribute *attr)
    在该设备的目录创建属性文件。
    dev: 要创建的属性文件的设备目录
    attr: 要创建的属性文件的上层 device_attribute 实例

13. void device_remove_file(struct device *dev, const struct device_attribute *attr)
    在该设备的目录移除属性文件.
    dev: 要移除的属性文件的设备目录
    attr: 要移除的属性文件的上层 device_attribute 实例

14. int device_create_bin_file(struct device *dev, const struct bin_attribute *attr)

15. void device_remove_bin_file(struct device *dev, const struct bin_attribute *attr)

16. struct kobject *virtual_device_parent(struct device *dev)
    在 /sys/devices/ 下创建一个 kobject, 名为 "virtual"
    dev: 未使用该参数

    返回值: 返回该 virtual 对应的 kobject, 这是个静态局部变量

    * 这个目录下的设备都有什么含义呢？
    
17. int dev_set_name(struct device *dev, const char *fmt, ...)
    设置 device 在 sysfs 下的目录名
    dev: 要设置目录名的设备.
    fmt: 名字

    返回值: 

18. void device_initialize(struct device *dev)
    初始化一个 device, 主要是:
    (1) 指定 device->kobj 的 kset, 该 kset 对应的目录是 /sys/devices/
    (2) 指定 device->kobj 的 ktype 为 device_ktype
    (3) 初始化各种锁, 链表头, 电源管理相关的

19. int device_add(struct device *dev)
    向内核添加一个初始化过的 device, 主要是
    (1) 增加 device 的引用计数, 其实就是增加 device->kobj 的引用计数, 表示当前 device 的目录被引用了一次 (当引用计数减小到0的时候, 会触发一连串的 ->release() 回调, 释放各种内存)
    (2) 申请该设备的私有结构, 即 device->p 成员 (struct device_private 类型)
    (3) 设置该设备的名字: 可以在 device_add()之前使用 dev_set_name()设置名字,或者使用 dev->init_name 设置名字, 二者中 dev->init_name 优先, 如果两者都未实现, 会使用该设备所属的 bus 名加上一个
        index 命名, 如果还没有所属的 bus, 那么 device_add() 将会失败
    (4) 选择该 device 的目录创建于哪个 sysfs 目录下, 稍后会指定 device 的 kobject 的父 kobject 为该目录的 kobject// 这里面门道较多，以后再总结
    (5) 调用 kobject_add() 添加 device 的 kobject 到内核并创建对应的目录
    (6) 在创建好的目录下创建 uevent 属性文件 // 关于该文件的读写，以后再研究
    (7) 如果该 device 对应一个设备树节点, 那么在该 device 目录下创建一个软链接, 指向该设备树节点对应的 sysfs 下的目录, 软链接名为 "of_node"
    (8) 如果该 device 属于某个 class:
        在该 device 目录下创建一个软链接, 指向 class->p->subsys.kobj 对应的目录, 软链接名为 "subsystem"
        对于非 partition 设备, 在该 device 目录下创建一个软链接, 指向父设备对应的目录, 软链接名为 "device"
        在该 device->class->p->subsys.kobj 目录下创建一个软链接, 指向 device 对应的目录, 软链接名为该 device 的名字
    (9) // 调用 device_add_attrs() 创建一些列的文件, 以后再研究吧
    (10) // 调用 bus_add_device() 创建一些列的文件, 以后再研究吧
        klist_add_tail(&dev->p->knode_bus, &bus->p->klist_devices);
    (11) // 调用 dpm_sysfs_add() device_pm_add() , 以后再研究吧
    (12) 如果该函数调用之前指定了设备号(即设备号不为 0):
            a. 在该设备的目录下创建 "dev" 属性文件, 可通过该文件查看设备号
            b. 在 "/sys/dev/char/" 目录下创建软链接，指向当前 device 的目录, 软链接名字格式为 "主设备号:子设备号"
            c. 创建该设备在 /dev 目录下的文件节点 // devtmpfs_create_node(dev);
    (13) // 如果设备有 bus, 调用 blocking_notifier_call_chain(&dev->bus->p->bus_notifier, BUS_NOTIFY_ADD_DEVICE, dev);
    (14) // 调用 kobject_uevent(&dev->kobj, KOBJ_ADD);
    (15) // 调用 bus_probe_device(dev);
    (16) 将该设备加入父设备为头的 klist 链表中, 也将该设备加入设备所属的类为头的 klist 链表中
    (17) 通知类中的其他设备, 有设备进行添加操作?
    (17) 减小 device 的引用计数

20. void device_del(struct device *dev)

21. int device_register(struct device *dev)
        device_initialize(dev);
        return device_add(dev);

22. void device_unregister(struct device *dev)

23. struct device *get_device(struct device *dev)
    增加该设备的引用计数, 本质是增加该设备对应的 kobject 的引用计数

24. void put_device(struct device *dev)
    减少该设备的引用计数, 本质是减少该设备对应的 kobject 的引用计数

25. const char *device_get_devnode(struct device *dev, umode_t *mode, kuid_t *uid, kgid_t *gid, const char **tmp)

26. int device_for_each_child(struct device *parent, void *data, int (*fn)(struct device *dev, void *data))
    遍历 device 下的所有子设备, 对每个子设备调用 fn() 函数, 只要一个 fn() 执行失败, 那么就返回了不再执行. 其中 data 是 fn() 函数的一个形参

27. struct device *device_find_child(struct device *parent, void *data, int (*match)(struct device *dev, void *data))
    对 device 下的所有子设备, 执行 match() 函数进行匹配, 匹配到了则返回对应子设备的实例

28. int device_offline(struct device *dev)

29. int device_online(struct device *dev)

20 static inline bool device_supports_offline(struct device *dev)
    return dev->bus && dev->bus->offline && dev->bus->online;

31. struct device *__root_device_register(const char *name, struct module *owner)
    #define root_device_register(name) __root_device_register(name, THIS_MODULE)

32. void root_device_unregister(struct device *dev)

33. struct device *device_create(struct class *class, struct device *parent, dev_t devt, void *drvdata, const char *fmt, ...)
    申请一个 device, 并调用 device_initialize() 进行初始化, 随后用形参填充部分成员, 最后调用 device_add() 向内核添加该设备

    * device_register() 和 device_create() 的区别就是: 调用 device_register() 之前, 先创建 device 实例并初始化部分成成员, 最后添加到内核, 而 device_create() 直接内部创建并初始化, 最后添加到内核

34. struct device *device_create_with_groups(struct class *class, struct device *parent, dev_t devt, void *drvdata, const struct attribute_group **groups, const char *fmt, ...)
    和 device_create() 的区别是, 在创建 device 的时候还会创建 groups 中指定的属性文件,其余作用一致

35. void device_destroy(struct class *class, dev_t devt)
    device_register() 和 device_create() 的相反作用

36. int device_rename(struct device *dev, const char *new_name)

37. int device_move(struct device *dev, struct device *new_parent, enum dpm_order dpm_order)

38. void device_shutdown(void)