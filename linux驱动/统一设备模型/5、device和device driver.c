1、linux 中的 device 和 device_driver


2、重要的结构体
    涉及文件: 
    include/linux/device.h
    drivers/base/base.h
    drivers/base/core.c

/**
 * struct device_driver - The basic device driver structure
 * @name:   Name of the device driver.
 * @bus:    The bus which the device of this driver belongs to.
 * @owner:  The module owner.
 * @mod_name:   Used for built-in modules.
 * @suppress_bind_attrs: Disables bind/unbind via sysfs.
 * @of_match_table: The open firmware table.
 * @acpi_match_table: The ACPI match table.
 * @probe:  Called to query the existence of a specific device,
 *      whether this driver can work with it, and bind the driver
 *      to a specific device.
 * @remove: Called when the device is removed from the system to
 *      unbind a device from this driver.
 * @shutdown:   Called at shut-down time to quiesce the device.
 * @suspend:    Called to put the device to sleep mode. Usually to a
 *      low power state.
 * @resume: Called to bring a device from sleep mode.
 * @groups: Default attributes that get created by the driver core
 *      automatically.
 * @pm:     Power management operations of the device which matched
 *      this driver.
 * @p:      Driver core's private data, no one other than the driver
 *      core can touch this.
 *
 * The device driver-model tracks all of the drivers known to the system.
 * The main reason for this tracking is to enable the driver core to match
 * up drivers with new devices. Once drivers are known objects within the
 * system, however, a number of other things become possible. Device drivers
 * can export information and configuration variables that are independent
 * of any specific device.
 */
struct device_driver {
    const char      *name; // 该driver的名称.
    struct bus_type     *bus; // 该driver所驱动设备的总线设备.为什么driver需要记录总线设备的指针呢？因为内核要保证在driver运行前, 设备所依赖的总线能够正确初始化.
    struct module       *owner; // 內核module相关的变量, 暂不描述.
    const char      *mod_name;  // 內核module相关的变量, 暂不描述./* used for built-in modules */
    bool suppress_bind_attrs;   // 是不在sysfs中启用bind和unbind attribute, 如下: /* disables bind/unbind via sysfs */
                                // root@android:/storage/sdcard0 # ls /sys/bus/platform/drivers/switch-gpio/                                                  
                                // bind   uevent unbind
                                // 在kernel中, bind/unbind是从用户空间手动的为driver绑定/解绑定指定的设备的机制.这种机制是在bus.c中完成的
    const struct of_device_id   *of_match_table;
    const struct acpi_device_id *acpi_match_table;
    int (*probe) (struct device *dev); // probe、remove, 这两个接口函数用于实现driver逻辑的开始和结束
    int (*remove) (struct device *dev);
    void (*shutdown) (struct device *dev); // shutdown、suspend、resume、pm, 电源管理相关的内容, 会在电源管理专题中详细说明.
    int (*suspend) (struct device *dev, pm_message_t state);
    int (*resume) (struct device *dev);
    const struct attribute_group **groups; // 和struct device结构中的同名变量类似, driver也可以定义一些默认attribute, 这样在将driver注册到内核中时, 内核设备模型
                                           // 部分的代码(driver/base/driver.c)会自动将这些attribute添加到sysfs中.
    const struct dev_pm_ops *pm;
    struct driver_private *p; // driver core的私有数据指针, 其它模块不能访问.
};
struct device_type { // device_type是内嵌在struct device结构中的一个数据结构, 用于指明设备的类型, 并提供一些额外的辅助功能.
    const char *name; // name表示该类型的名称, 当该类型的设备添加到内核时, 内核会发出"DEVTYPE=‘name’”类型的uevent, 告知用户空间某个类型的设备available了
    const struct attribute_group **groups; // 该类型设备的公共attribute集合.设备注册时, 会同时注册这些attribute.
    int (*uevent)(struct device *dev, struct kobj_uevent_env *env); // 所有相同类型的设备, 会有一些共有的uevent需要发送, 由该接口实现
    char *(*devnode)(struct device *dev, umode_t *mode, kuid_t *uid, kgid_t *gid); // devtmpfs有关的内容, 暂不说明
    void (*release)(struct device *dev); // device_release()会使用到, 如果device结构没有提供release接口, 就要查询它所属的type是否提供.用于释放device变量所占的空间
    const struct dev_pm_ops *pm;
};

3、设备模型框架下驱动开发的基本步骤
    步骤1: 分配一个struct device类型的变量, 填充必要的信息后, 把它注册到内核中.
    步骤2: 分配一个struct device_driver类型的变量, 填充必要的信息后, 把它注册到内核中.
    这两步完成后, 内核会在合适的时机(后面会讲), 调用struct device_driver变量中的probe、remove、suspend、resume等回调函数, 从而触发或者终结设备驱动的执行.而所有的
驱动程序逻辑, 都会由这些回调函数实现, 此时, 驱动开发者眼中便不再有“设备模型”, 转而只关心驱动本身的实现.

    一般情况下, Linux驱动开发很少直接使用device和device_driver, 因为内核在它们之上又封装了一层, 如soc device、platform device等等, 而这些层次提供的接口更为简单、易用

4、设备驱动probe的时机
    所谓的"probe", 是指内核根据某种匹配方式为驱动添加设备, 或者为设备添加驱动时, 内核就会执行device_driver中的probe回调函数, 而该函数就是所有driver的入口, 可以执行诸如
硬件设备初始化、字符设备注册、设备文件操作ops注册等动作("remove"是它的反操作, 发生在device或者device_driver任何一方从内核注销时).
    触发设备匹配驱动或驱动匹配设备的时机: 
    (1)将struct device类型的变量注册到内核中时自动触发(device_register, device_add, device_create_vargs, device_create) // 向内核添加设备
    (2)将struct device_driver类型的变量注册到内核中时自动触发(driver_register)                                         // 向内核添加驱动
    (3)手动查找同一bus下的所有device_driver, 如果有和指定device同名的driver, 执行probe操作(device_attach)
    (4)手动查找同一bus下的所有device, 如果有和指定driver同名的device, 执行probe操作(driver_attach)
    (5)自行调用driver的probe接口, 并在该接口中将该driver绑定到某个device结构中----即设置dev->driver(device_bind_driver)

    实际上, probe动作实际是由bus模块实现的, 每个bus都有一个drivers_autoprobe变量, 用于控制是否在device或者driver注册时, 自动probe.该变量默认为1(即自动probe), bus模
块将它开放到sysfs中了, 因而可在用户空间修改, 进而控制probe行为.