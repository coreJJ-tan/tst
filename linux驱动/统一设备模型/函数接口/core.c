1. const char *dev_driver_string(const struct device *dev)
    获取设备绑定的驱动的名字
    dev: 要获取的设备结构体

    返回值：设备的驱动名 或者 绑定的总线名 或者 设备的 class 名 或者 ""

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

5. ssize_t device_show_ulong(struct device *dev, struct device_attribute *attr, char *buf)

6. ssize_t device_store_int(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)

7. ssize_t device_show_int(struct device *dev, struct device_attribute *attr, char *buf)

8. ssize_t device_store_bool(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)

9. ssize_t device_show_bool(struct device *dev, struct device_attribute *attr, char *buf)