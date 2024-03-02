struct dev_ext_attribute {
    struct device_attribute attr;
    void *var;
};
/**
 * struct device - The basic device structure
 * @parent: The device's "parent" device, the device to which it is attached.
 *      In most cases, a parent device is some sort of bus or host
 *      controller. If parent is NULL, the device, is a top-level device,
 *      which is not usually what you want.
 * @p:      Holds the private data of the driver core portions of the device.
 *      See the comment of the struct device_private for detail.
 * @kobj:   A top-level, abstract class from which other classes are derived.
 * @init_name:  Initial name of the device.
 * @type:   The type of device.
 *      This identifies the device type and carries type-specific
 *      information.
 * @mutex:  Mutex to synchronize calls to its driver.
 * @bus:    Type of bus device is on.
 * @driver: Which driver has allocated this
 * @platform_data: Platform data specific to the device.
 *      Example: For devices on custom boards, as typical of embedded
 *      and SOC based hardware, Linux often uses platform_data to point
 *      to board-specific structures describing devices and how they
 *      are wired.  That can include what ports are available, chip
 *      variants, which GPIO pins act in what additional roles, and so
 *      on.  This shrinks the "Board Support Packages" (BSPs) and
 *      minimizes board-specific #ifdefs in drivers.
 * @driver_data: Private pointer for driver specific info.
 * @power:  For device power management.
 *      See Documentation/power/devices.txt for details.
 * @pm_domain:  Provide callbacks that are executed during system suspend,
 *      hibernation, system resume and during runtime PM transitions
 *      along with subsystem-level and driver-level callbacks.
 * @pins:   For device pin management.
 *      See Documentation/pinctrl.txt for details.
 * @numa_node:  NUMA node this device is close to.
 * @dma_mask:   Dma mask (if dma'ble device).
 * @coherent_dma_mask: Like dma_mask, but for alloc_coherent mapping as not all
 *      hardware supports 64-bit addresses for consistent allocations
 *      such descriptors.
 * @dma_pfn_offset: offset of DMA memory range relatively of RAM
 * @dma_parms:  A low level driver may set these to teach IOMMU code about
 *      segment limitations.
 * @dma_pools:  Dma pools (if dma'ble device).
 * @dma_mem:    Internal for coherent mem override.
 * @cma_area:   Contiguous memory area for dma allocations
 * @archdata:   For arch-specific additions.
 * @of_node:    Associated device tree node.
 * @fwnode: Associated device node supplied by platform firmware.
 * @devt:   For creating the sysfs "dev".
 * @id:     device instance
 * @devres_lock: Spinlock to protect the resource of the device.
 * @devres_head: The resources list of the device.
 * @knode_class: The node used to add the device to the class list.
 * @class:  The class of the device.
 * @groups: Optional attribute groups.
 * @release:    Callback to free the device after all references have
 *      gone away. This should be set by the allocator of the
 *      device (i.e. the bus driver that discovered the device).
 * @iommu_group: IOMMU group the device belongs to.
 *
 * @offline_disabled: If set, the device is permanently online.
 * @offline:    Set after successful invocation of bus type's .offline().
 *
 * At the lowest level, every device in a Linux system is represented by an
 * instance of struct device. The device structure contains the information
 * that the device model core needs to model the system. Most subsystems,
 * however, track additional information about the devices they host. As a
 * result, it is rare for devices to be represented by bare device structures;
 * instead, that structure, like kobject structures, is usually embedded within
 * a higher-level representation of the device.
 */
struct device {
    struct device       *parent; // 该设备的父设备, 一般是该设备所从属的bus、controller等设备.
    struct device_private   *p; // 一个用于struct device的私有数据结构指针, 该指针中会保存子设备链表、用于添加到bus/driver/prent等设备中的链表头等等.
    struct kobject kobj;
    const char      *init_name; //该设备的默认名称, 只在device_add()刚添加设备时会起作用, 可以使用 dev_set_name()设置设备名字,sysfs下设备的名字其实就是它的 kobject 的名字,最终设置的是kobject的名字
                                // 在device_add()之后,该成员会指向NULL
    const struct device_type *type;
    struct mutex        mutex;  /* mutex to synchronize calls to its driver. */
    struct bus_type *bus;       // 该device属于哪个总线/* type of bus device is on */
    struct device_driver *driver;   // 该device对应的device driver./* which driver has allocated this device */
    void *platform_data; // 一个指针, 用于保存具体的平台相关的数据.具体的driver模块, 可以将一些私有的数据, 暂存在这里, 需要使用的时候, 再拿出来, 因此设备模型并不关心该指针得实际含义./* Platform specific data, device core doesn't touch it */
    void *driver_data;   /* Driver data, set and get with dev_set/get_drvdata */
    struct dev_pm_info  power; // 电源管理相关的逻辑
    struct dev_pm_domain    *pm_domain; // 电源管理相关的逻辑
#ifdef CONFIG_PINCTRL
    struct dev_pin_info *pins; // "PINCTRL”功能, 暂不描述.
#endif
#ifdef CONFIG_NUMA
    int     numa_node;  // "NUMA”功能, 暂不描述./* NUMA node this device is close to */
#endif
    u64     *dma_mask;  // DMA相关的功能, 暂不描述./* dma mask (if dma'able device) */
    u64     coherent_dma_mask; // DMA相关的功能, 暂不描述./* Like dma_mask, but for alloc_coherent mappings as not all hardware supports 64 bit addresses for consistent allocations such descriptors. */
    unsigned long   dma_pfn_offset; // DMA相关的功能, 暂不描述.
    struct device_dma_parameters *dma_parms; // DMA相关的功能, 暂不描述.
    struct list_head    dma_pools;  // DMA相关的功能, 暂不描述./* dma pools (if dma'ble) */
    struct dma_coherent_mem *dma_mem; // DMA相关的功能, 暂不描述. /* internal for coherent mem override */
#ifdef CONFIG_DMA_CMA
    struct cma *cma_area;  // DMA相关的功能, 暂不描述.     /* contiguous memory area for dma allocations */
#endif
    struct dev_archdata archdata; // DMA相关的功能, 暂不描述.
    struct device_node  *of_node; /* associated device tree node */
    struct fwnode_handle    *fwnode; /* firmware device node */
    dev_t           devt;   // 设备号, dev_t是一个32位的整数, 它由两个部分(Major和Minor)组成/* dev_t, creates the sysfs "dev" */
    u32         id; /* device instance */
    spinlock_t      devres_lock;
    struct list_head    devres_head;
    struct klist_node   knode_class;
    struct class        *class; // 该设备属于哪个class.
    const struct attribute_group **groups;  // 该设备的默认attribute集合.将会在设备注册时自动在sysfs中创建对应的文件./* optional groups */
    void    (*release)(struct device *dev); // device_release()会使用到, 如果定义, 对应的kobject引用计数为0时会调用
    struct iommu_group  *iommu_group;
    bool            offline_disabled:1;
    bool            offline:1;  // true 表示设备处于 offline 状态
};

/*
 * The type of device, "struct device" is embedded in. A class
 * or bus can contain devices of different types
 * like "partitions" and "disks", "mouse" and "event".
 * This identifies the device type and carries type-specific
 * information, equivalent to the kobj_type of a kobject.
 * If "name" is specified, the uevent will contain it in
 * the DEVTYPE variable.
 */
struct device_type {
    const char *name;
    const struct attribute_group **groups;
    int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
    char *(*devnode)(struct device *dev, umode_t *mode,
             kuid_t *uid, kgid_t *gid);
    void (*release)(struct device *dev);

    const struct dev_pm_ops *pm;
};

/**
 * struct device_private - structure to hold the private to the driver core portions of the device structure.
 *
 * @klist_children - klist containing all children of this device
 * @knode_parent - node in sibling list
 * @knode_driver - node in driver list
 * @knode_bus - node in bus list
 * @deferred_probe - entry in deferred_probe_list which is used to retry the
 *  binding of drivers which were unable to get all the resources needed by
 *  the device; typically because it depends on another driver getting
 *  probed first.
 * @device - pointer back to the struct class that this structure is
 * associated with.
 *
 * Nothing outside of the driver core should ever touch these fields.
 */
struct device_private {
    struct klist klist_children;
    struct klist_node knode_parent;
    struct klist_node knode_driver;
    struct klist_node knode_bus;
    struct list_head deferred_probe;
    struct device *device;
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

struct driver_private {
    struct kobject kobj; // 该设备驱动对应的 kobject
    struct klist klist_devices;
    struct klist_node knode_bus;
    struct module_kobject *mkobj;
    struct device_driver *driver;
};

struct driver_attribute {
	struct attribute attr;
	ssize_t (*show)(struct device_driver *driver, char *buf);
	ssize_t (*store)(struct device_driver *driver, const char *buf,
			 size_t count);
};


/**
 * struct bus_type - The bus type of the device
 *
 * @name:	The name of the bus.
 * @dev_name:	Used for subsystems to enumerate devices like ("foo%u", dev->id).
 * @dev_root:	Default device to use as the parent.
 * @dev_attrs:	Default attributes of the devices on the bus.
 * @bus_groups:	Default attributes of the bus.
 * @dev_groups:	Default attributes of the devices on the bus.
 * @drv_groups: Default attributes of the device drivers on the bus.
 * @match:	Called, perhaps multiple times, whenever a new device or driver
 *		is added for this bus. It should return a nonzero value if the
 *		given device can be handled by the given driver.
 * @uevent:	Called when a device is added, removed, or a few other things
 *		that generate uevents to add the environment variables.
 * @probe:	Called when a new device or driver add to this bus, and callback
 *		the specific driver's probe to initial the matched device.
 * @remove:	Called when a device removed from this bus.
 * @shutdown:	Called at shut-down time to quiesce the device.
 *
 * @online:	Called to put the device back online (after offlining it).
 * @offline:	Called to put the device offline for hot-removal. May fail.
 *
 * @suspend:	Called when a device on this bus wants to go to sleep mode.
 * @resume:	Called to bring a device on this bus out of sleep mode.
 * @pm:		Power management operations of this bus, callback the specific
 *		device driver's pm-ops.
 * @iommu_ops:  IOMMU specific operations for this bus, used to attach IOMMU
 *              driver implementations to a bus and allow the driver to do
 *              bus-specific setup
 * @p:		The private data of the driver core, only the driver core can
 *		touch this.
 * @lock_key:	Lock class key for use by the lock validator
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
	const char		*name;
	const char		*dev_name;
	struct device		*dev_root; // 如果该总线被当成一个设备，那么该成员就指向了它的设备结构体（参见 subsys_register()函数）
	struct device_attribute	*dev_attrs;	// 这是一个数组, 所有挂在该总线上的设备都会在 bus_add_device() 阶段创建这个数组中的所有属性文件, 这个数组的最后一个成员要保持为 __ATTR_NULL /* use dev_groups instead */
	const struct attribute_group **bus_groups; // 这是一个数组, 该成员指定的所有属性文件将会在 bus_register() 函数中被创建于总线的目录下
	const struct attribute_group **dev_groups; // 这是一个数组, 所有挂在该总线上的设备都会在 bus_add_device() 阶段创建这个数组中的所有组的属性文件
	const struct attribute_group **drv_groups;

	int (*match)(struct device *dev, struct device_driver *drv);
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
struct bus_attribute {
    struct attribute    attr;
    ssize_t (*show)(struct bus_type *bus, char *buf);
    ssize_t (*store)(struct bus_type *bus, const char *buf, size_t count);
};
struct subsys_interface {
	const char *name;
	struct bus_type *subsys; // 所属的总线
	struct list_head node; // 通过该节点挂在 subsys->p->interfaces 链表之上
	int (*add_dev)(struct device *dev, struct subsys_interface *sif);
	int (*remove_dev)(struct device *dev, struct subsys_interface *sif);
};

/**
 * struct class - device classes
 * @name:	Name of the class.
 * @owner:	The module owner.
 * @class_attrs: Default attributes of this class.
 * @dev_groups:	Default attributes of the devices that belong to the class.
 * @dev_kobj:	The kobject that represents this class and links it into the hierarchy.
 * @dev_uevent:	Called when a device is added, removed from this class, or a
 *		few other things that generate uevents to add the environment
 *		variables.
 * @devnode:	Callback to provide the devtmpfs.
 * @class_release: Called to release this class.
 * @dev_release: Called to release the device.
 * @suspend:	Used to put the device to sleep mode, usually to a low power
 *		state.
 * @resume:	Used to bring the device from the sleep mode.
 * @ns_type:	Callbacks so sysfs can detemine namespaces.
 * @namespace:	Namespace of the device belongs to this class.
 * @pm:		The default device power management operations of this class.
 * @p:		The private data of the driver core, no one other than the
 *		driver core can touch this.
 *
 * A class is a higher-level view of a device that abstracts out low-level
 * implementation details. Drivers may see a SCSI disk or an ATA disk, but,
 * at the class level, they are all simply disks. Classes allow user space
 * to work with devices based on what they do, rather than how they are
 * connected or how they work.
 */
struct class {
	const char		*name;
	struct module		*owner;

	struct class_attribute		*class_attrs; // 该类在注册时创建的一些属性文件, 指向一个数组, 数组最后一个成员保持为 __ATTR_NULL 
	const struct attribute_group	**dev_groups;
	struct kobject			*dev_kobj; // 对应 /sys/dev/char 或者 /sys/dev/block,注册class时如果未指定，那么默认为 /sys/dev/char

	int (*dev_uevent)(struct device *dev, struct kobj_uevent_env *env);
	char *(*devnode)(struct device *dev, umode_t *mode);

	void (*class_release)(struct class *class);
	void (*dev_release)(struct device *dev);

	int (*suspend)(struct device *dev, pm_message_t state);
	int (*resume)(struct device *dev);

	const struct kobj_ns_type_operations *ns_type;
	const void *(*namespace)(struct device *dev);

	const struct dev_pm_ops *pm;

	struct subsys_private *p;
};
struct class_attribute {
	struct attribute attr;
	ssize_t (*show)(struct class *class, struct class_attribute *attr, char *buf);
	ssize_t (*store)(struct class *class, struct class_attribute *attr, const char *buf, size_t count);
};

/**
 * struct subsys_private - structure to hold the private to the driver core portions of the bus_type/class structure.
 *
 * @subsys - the struct kset that defines this subsystem
 * @devices_kset - the subsystem's 'devices' directory
 * @interfaces - list of subsystem interfaces associated
 * @mutex - protect the devices, and interfaces lists.
 *
 * @drivers_kset - the list of drivers associated
 * @klist_devices - the klist to iterate over the @devices_kset
 * @klist_drivers - the klist to iterate over the @drivers_kset
 * @bus_notifier - the bus notifier list for anything that cares about things
 *                 on this bus.
 * @bus - pointer back to the struct bus_type that this structure is associated
 *        with.
 *
 * @glue_dirs - "glue" directory to put in-between the parent device to
 *              avoid namespace conflicts
 * @class - pointer back to the struct class that this structure is associated
 *          with.
 *
 * This structure is the one that is the actual kobject allowing struct
 * bus_type/class to be statically allocated safely.  Nothing outside of the
 * driver core should ever touch these fields.
 */
struct subsys_private { // 这个结构体被 bus_type/class 两个结构体包含, 代表这两个大结构体的私有部分
    struct kset subsys; // 代表 bus_type/class 自己子系统的 kset，会根据这个成员的 subsys->kobj->entry 链表连接所有的 bus/class ?
    struct kset *devices_kset; // 该 bus_type/class 下的设备的共有 kset，其下的设备会以 devices_kset-> list 作为头部, 将它们 kobject 链接到这个链表头上
    struct list_head interfaces; // 链表头部，链接的节点是 subsys_interface->node
    struct mutex mutex;

    struct kset *drivers_kset; // 该 bus_type/class 下的驱动的共有 kset，其下的驱动会以 devices_kset-> list 作为头部, 将它们 kobject 链接到这个链表头上
    struct klist klist_devices; // klist 链表的头部，其下会链接多个设备
    struct klist klist_drivers; // klist 链表的头部，其下会链接多个驱动
    struct blocking_notifier_head bus_notifier; // 该总线的通知链表头，挂在总线上的设备或者驱动可以注册通知链挂到上面，用于监听总线上发生的事（通过bus_unregister_notifier()注册）
    unsigned int drivers_autoprobe:1; /*drivers_autoprobe是一个bit变量,为l则允许本条总线上的device注册时自动匹配driver,drivers_autoprobe默认总是为1,除非用户空间修改*/
    struct bus_type *bus;

    struct kset glue_dirs;
    struct class *class;
};