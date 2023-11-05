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
    在该设备的目录移除属性文件。
    dev: 要移除的属性文件的设备目录
    attr: 要移除的属性文件的上层 device_attribute 实例

14. int device_create_bin_file(struct device *dev, const struct bin_attribute *attr)

15. void device_remove_bin_file(struct device *dev, const struct bin_attribute *attr)

16. void device_initialize(struct device *dev)
    初始化一个 device, 主要是:
    (1) 指定 device->kobj 的 kset, 该 kset 对应的目录是 /sys/devices/
    (2) 指定 device->kobj 的 ktype 为 device_ktype
    (3) 初始化各种锁, 链表头, 电源管理相关的

17. struct kobject *virtual_device_parent(struct device *dev)