/**
     * struct bus_type - The bus type of the device
     *
     * @name:   The name of the bus.
     * @dev_name:   Used for subsystems to enumerate devices like ("foo%u", dev->id).
     * @dev_root:   Default device to use as the parent.
     * @dev_attrs:  Default attributes of the devices on the bus.
     * @bus_groups: Default attributes of the bus.
     * @dev_groups: Default attributes of the devices on the bus.
     * @drv_groups: Default attributes of the device drivers on the bus.
     * @match:  Called, perhaps multiple times, whenever a new device or driver
     *      is added for this bus. It should return a nonzero value if the
     *      given device can be handled by the given driver.
     * @uevent: Called when a device is added, removed, or a few other things
     *      that generate uevents to add the environment variables.
     * @probe:  Called when a new device or driver add to this bus, and callback
     *      the specific driver's probe to initial the matched device.
     * @remove: Called when a device removed from this bus.
     * @shutdown:   Called at shut-down time to quiesce the device.
     *
     * @online: Called to put the device back online (after offlining it).
     * @offline:    Called to put the device offline for hot-removal. May fail.
     *
     * @suspend:    Called when a device on this bus wants to go to sleep mode.
     * @resume: Called to bring a device on this bus out of sleep mode.
     * @pm:     Power management operations of this bus, callback the specific
     *      device driver's pm-ops.
     * @iommu_ops:  IOMMU specific operations for this bus, used to attach IOMMU
     *              driver implementations to a bus and allow the driver to do
     *              bus-specific setup
     * @p:      The private data of the driver core, only the driver core can
     *      touch this.
     * @lock_key:   Lock class key for use by the lock validator
     *
     * A bus is a channel between the processor and one or more devices. For the
     * purposes of the device model, all devices are connected via a bus, even if
     * it is an internal, virtual, "platform" bus. Buses can plug into each other.
     * A USB controller is usually a PCI device, for example. The device model
     * represents the actual connections between buses and the devices they control.
     * A bus is represented by the bus_type structure. It contains the name, the
     * default attributes, the bus' methods, PM operations, and the driver core's
     * private data.
 */
struct bus_type {
    const char      *name;  /* 总线名字 */
    const char      *dev_name;
    struct device       *dev_root;
    struct device_attribute *dev_attrs; /* use dev_groups instead */
    const struct attribute_group **bus_groups;      /* 总线属性 */
    const struct attribute_group **dev_groups;      /* 设备属性 */ // 一个总线定义了这么一个属性 groups，那么当device_add()之时，该总线下的设备目录中就会创建这些groups中的属性文件
    const struct attribute_group **drv_groups;      /* 驱动属性 */

    int (*match)(struct device *dev, struct device_driver *drv);    // 此函数就是完成设备和驱动之间匹配的，总线就是使用 match 函数来根据注册的驱动来查找对应的设备，
                                                                    // 因此每一条总线都必须实现此函数。
                                                                    // match 函数有两个参数： dev 和 drv，这两个参数分别为 device 和 device_driver 类型，也就是设备和驱动。 
    int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
    int (*probe)(struct device *dev);
    int (*remove)(struct device *dev);
    void (*shutdown)(struct device *dev);

    int (*online)(struct device *dev);
    int (*offline)(struct device *dev);

    int (*suspend)(struct device *dev, pm_message_t state);
    int (*resume)(struct device *dev);

    const struct dev_pm_ops *pm;

    const struct iommu_ops *iommu_ops;

    struct subsys_private *p;
    struct lock_class_key lock_key;
};

struct bus_type platform_bus_type = {
    .name       = "platform",
    .dev_groups = platform_dev_groups,
    .match      = platform_match,   // platform 总线驱动与设备匹配的函数
    .uevent     = platform_uevent,
    .pm     = &platform_dev_pm_ops,
};
EXPORT_SYMBOL_GPL(platform_bus_type);


struct platform_driver {
    int (*probe)(struct platform_device *);      // 当驱动与设备匹配成功以后 probe 函数就会执行，因此可以将添加设备节点等操作放置该函数中
    int (*remove)(struct platform_device *);
    void (*shutdown)(struct platform_device *);
    int (*suspend)(struct platform_device *, pm_message_t state);
    int (*resume)(struct platform_device *);
    struct device_driver driver;
    const struct platform_device_id *id_table;
    bool prevent_deferred_probe;
};

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
    const char      *name;
    struct bus_type     *bus;

    struct module       *owner;
    const char      *mod_name;  /* used for built-in modules */

    bool suppress_bind_attrs;   /* disables bind/unbind via sysfs */

    const struct of_device_id   *of_match_table;
    const struct acpi_device_id *acpi_match_table;

    int (*probe) (struct device *dev);
    int (*remove) (struct device *dev);
    void (*shutdown) (struct device *dev);
    int (*suspend) (struct device *dev, pm_message_t state);
    int (*resume) (struct device *dev);
    const struct attribute_group **groups;

    const struct dev_pm_ops *pm;

    struct driver_private *p;
};

struct of_device_id {
    char    name[32];
    char    type[32];
    char    compatible[128];    // 
    const void *data;
};

// ===================================================================================================================================
platform 总线设备与驱动匹配的几种方式：
static int platform_match(struct device *dev, struct device_driver *drv)
    struct platform_device *pdev = to_platform_device(dev);
    struct platform_driver *pdrv = to_platform_driver(drv);
    if (pdev->driver_override)  // 可以设置 platform_device 的 driver_override，强制选择某个 platform_driver。
        return !strcmp(pdev->driver_override, drv->name);       /* When driver_override is set, only bind to the matching driver */
    if (of_driver_match_device(dev, drv))           /* Attempt an OF style match first */
        return 1; // 有设备树的匹配方式，对于4.1.15版本的内核，主要是根据设备树节点中的compatible属性与驱动的 of_match_table 数组成员进行匹配的，
                  // 对于4.19版本以上的内核，有多个一样的 compatible 属性的节点，会根据 of_match_table 的其他成员 name 和 type 进一步匹配，找到最优的匹配
    if (acpi_driver_match_device(dev, drv))         /* Then try ACPI style match */
        return 1;
    if (pdrv->id_table)     
        return platform_match_id(pdrv->id_table, pdev) != NULL;     /* Then try to match against the id table */
    return (strcmp(pdev->name, drv->name) == 0);   //最后没有设备树的方式，直接根据设备名与驱动名进行匹配 /* fall-back to driver name match */
// ===================================================================================================================================
定义并初始化好 platform_driver 结构体变量以后，需要在驱动入口函数里面调用 platform_driver_register() 函数向 Linux 内核注册一个 platform 驱动:
int platform_driver_register (struct platform_driver *driver)   // 在该函数内部，会执行钩子函数对应的 platform_match() 函数，匹配到设备后，就会执行驱动的 probe()函数