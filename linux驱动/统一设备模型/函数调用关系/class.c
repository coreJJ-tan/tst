1. 初始化部分
    int __init classes_init(void)
        class_kset = kset_create_and_add("class", NULL, NULL); // 创建一个kset, 体现为 /sys/class 目录
        return 0;

2. int class_create_file_ns(struct class *cls, const struct class_attribute *attr, const void *ns)
    在类 cls 目录 /sys/class/<class_name> 下创建属性文件, ns 为命名空间

3. void class_remove_file_ns(struct class *cls, const struct class_attribute *attr, const void *ns)
    删除命名空间 ns 的类 cls 下的属性文件

4. int __class_register(struct class *cls, struct lock_class_key *key)
    注册类 cls // key 和加密有关, 暂时忽略

    * 初始化 class 下的设备 klist 链表, 其它锁, 链表等
    * 创建该 class 对应的 kset, 对应 /sys/class/<类名> 目录
    * 创建该类指定的文件, 由 cls->class_attrs 成员指定, 该成员的最后一个成员要保持为 __ATTR_NULL

5. class_register(class) 宏
    static struct lock_class_key __key;
    __class_register(class, &__key);

6. void class_unregister(struct class *cls)
    注销类 cls

    * 删除类 cls 在注册时创建的默认的属性文件
    * 注销该类对应的 kset

7. struct class *__class_create(struct module *owner, const char *name, struct lock_class_key *key)
    创建一个类并注册

    * 申请一个类的内存
    * 调用 __class_register() 注册类
    * 调用该函数后, 后续将该类作为 device_create() 的形参

8. class_create(owner, name) 宏
    static struct lock_class_key __key;
    __class_create(owner, name, &__key);

9. void class_destroy(struct class *cls)
    class_unregister(cls);

10. void class_dev_iter_init(struct class_dev_iter *iter, struct class *class, struct device *start, const struct device_type *type)
    struct device *class_dev_iter_next(struct class_dev_iter *iter)
    void class_dev_iter_exit(struct class_dev_iter *iter)

11. int class_for_each_device(struct class *class, struct device *start, void *data, int (*fn)(struct device *, void *))

12. struct device *class_find_device(struct class *class, struct device *start, const void *data, int (*match)(struct device *, const void *))

13. int class_interface_register(struct class_interface *class_intf)

14. void class_interface_unregister(struct class_interface *class_intf)

15. ssize_t show_class_attr_string(struct class *class, struct class_attribute *attr, char *buf)

16. struct class_compat *class_compat_register(const char *name)

17. void class_compat_unregister(struct class_compat *cls)

18. int class_compat_create_link(struct class_compat *cls, struct device *dev, struct device *device_link)

19. void class_compat_remove_link(struct class_compat *cls, struct device *dev, struct device *device_link)