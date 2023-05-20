1、linux 中的 device 和 device_driver


2、重要的结构体
    涉及文件：
    include/linux/device.h
    drivers/base/base.h
    drivers/base/core.c

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
    struct device       *parent; // 该设备的父设备，一般是该设备所从属的bus、controller等设备。
    struct device_private   *p; // 一个用于struct device的私有数据结构指针，该指针中会保存子设备链表、用于添加到bus/driver/prent等设备中的链表头等等。
    struct kobject kobj;
    const char      *init_name; //该设备的名称 /* initial name of the device */ 
    const struct device_type *type;
    struct mutex        mutex;  /* mutex to synchronize calls to its driver. */
    struct bus_type *bus;       // 该device属于哪个总线/* type of bus device is on */
    struct device_driver *driver;   // 该device对应的device driver。/* which driver has allocated this device */
    void *platform_data; // 一个指针，用于保存具体的平台相关的数据。具体的driver模块，可以将一些私有的数据，暂存在这里，需要使用的时候，再拿出来，因此设备模型并不关心该指针得实际含义。/* Platform specific data, device core doesn't touch it */
    void *driver_data;   /* Driver data, set and get with dev_set/get_drvdata */
    struct dev_pm_info  power; // 电源管理相关的逻辑
    struct dev_pm_domain    *pm_domain; // 电源管理相关的逻辑
#ifdef CONFIG_PINCTRL
    struct dev_pin_info *pins; // "PINCTRL”功能，暂不描述。
#endif
#ifdef CONFIG_NUMA
    int     numa_node;  // "NUMA”功能，暂不描述。/* NUMA node this device is close to */
#endif
    u64     *dma_mask;  // DMA相关的功能，暂不描述。/* dma mask (if dma'able device) */
    u64     coherent_dma_mask; // DMA相关的功能，暂不描述。/* Like dma_mask, but for alloc_coherent mappings as not all hardware supports 64 bit addresses for consistent allocations such descriptors. */
    unsigned long   dma_pfn_offset; // DMA相关的功能，暂不描述。
    struct device_dma_parameters *dma_parms; // DMA相关的功能，暂不描述。
    struct list_head    dma_pools;  // DMA相关的功能，暂不描述。/* dma pools (if dma'ble) */
    struct dma_coherent_mem *dma_mem; // DMA相关的功能，暂不描述。 /* internal for coherent mem override */
#ifdef CONFIG_DMA_CMA
    struct cma *cma_area;  // DMA相关的功能，暂不描述。     /* contiguous memory area for dma allocations */
#endif
    struct dev_archdata archdata; // DMA相关的功能，暂不描述。
    struct device_node  *of_node; /* associated device tree node */
    struct fwnode_handle    *fwnode; /* firmware device node */
    dev_t           devt;   // 设备号，dev_t是一个32位的整数，它由两个部分（Major和Minor）组成/* dev_t, creates the sysfs "dev" */
    u32         id; /* device instance */
    spinlock_t      devres_lock;
    struct list_head    devres_head;
    struct klist_node   knode_class;
    struct class        *class; // 该设备属于哪个class。
    const struct attribute_group **groups;  // 该设备的默认attribute集合。将会在设备注册时自动在sysfs中创建对应的文件。/* optional groups */
    void    (*release)(struct device *dev);
    struct iommu_group  *iommu_group;
    bool            offline_disabled:1;
    bool            offline:1;
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
    const char      *name; // 该driver的名称。
    struct bus_type     *bus; // 该driver所驱动设备的总线设备。为什么driver需要记录总线设备的指针呢？因为内核要保证在driver运行前，设备所依赖的总线能够正确初始化。
    struct module       *owner; // 內核module相关的变量，暂不描述。
    const char      *mod_name;  // 內核module相关的变量，暂不描述。/* used for built-in modules */
    bool suppress_bind_attrs;   // 是不在sysfs中启用bind和unbind attribute，如下：/* disables bind/unbind via sysfs */
                                // root@android:/storage/sdcard0 # ls /sys/bus/platform/drivers/switch-gpio/                                                  
                                // bind   uevent unbind
                                // 在kernel中，bind/unbind是从用户空间手动的为driver绑定/解绑定指定的设备的机制。这种机制是在bus.c中完成的
    const struct of_device_id   *of_match_table;
    const struct acpi_device_id *acpi_match_table;
    int (*probe) (struct device *dev); // probe、remove，这两个接口函数用于实现driver逻辑的开始和结束
    int (*remove) (struct device *dev);
    void (*shutdown) (struct device *dev); // shutdown、suspend、resume、pm，电源管理相关的内容，会在电源管理专题中详细说明。
    int (*suspend) (struct device *dev, pm_message_t state);
    int (*resume) (struct device *dev);
    const struct attribute_group **groups; // 和struct device结构中的同名变量类似，driver也可以定义一些默认attribute，这样在将driver注册到内核中时，内核设备模型
                                           // 部分的代码（driver/base/driver.c）会自动将这些attribute添加到sysfs中。
    const struct dev_pm_ops *pm;
    struct driver_private *p; // driver core的私有数据指针，其它模块不能访问。
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
struct device_type { // device_type是内嵌在struct device结构中的一个数据结构，用于指明设备的类型，并提供一些额外的辅助功能。
    const char *name; // name表示该类型的名称，当该类型的设备添加到内核时，内核会发出"DEVTYPE=‘name’”类型的uevent，告知用户空间某个类型的设备available了
    const struct attribute_group **groups; // 该类型设备的公共attribute集合。设备注册时，会同时注册这些attribute。
    int (*uevent)(struct device *dev, struct kobj_uevent_env *env); // 所有相同类型的设备，会有一些共有的uevent需要发送，由该接口实现
    char *(*devnode)(struct device *dev, umode_t *mode, kuid_t *uid, kgid_t *gid); // devtmpfs有关的内容，暂不说明
    void (*release)(struct device *dev); // 如果device结构没有提供release接口，就要查询它所属的type是否提供。用于释放device变量所占的空间
    const struct dev_pm_ops *pm;
};

3、设备模型框架下驱动开发的基本步骤
    步骤1：分配一个struct device类型的变量，填充必要的信息后，把它注册到内核中。
    步骤2：分配一个struct device_driver类型的变量，填充必要的信息后，把它注册到内核中。
    这两步完成后，内核会在合适的时机（后面会讲），调用struct device_driver变量中的probe、remove、suspend、resume等回调函数，从而触发或者终结设备驱动的执行。而所有的
驱动程序逻辑，都会由这些回调函数实现，此时，驱动开发者眼中便不再有“设备模型”，转而只关心驱动本身的实现。

    一般情况下，Linux驱动开发很少直接使用device和device_driver，因为内核在它们之上又封装了一层，如soc device、platform device等等，而这些层次提供的接口更为简单、易用

4、设备驱动probe的时机
    所谓的"probe"，是指内核根据某种匹配方式为驱动添加设备，或者为设备添加驱动时，内核就会执行device_driver中的probe回调函数，而该函数就是所有driver的入口，可以执行诸如
硬件设备初始化、字符设备注册、设备文件操作ops注册等动作（"remove"是它的反操作，发生在device或者device_driver任何一方从内核注销时）。
    触发设备匹配驱动或驱动匹配设备的时机：
    （1）将struct device类型的变量注册到内核中时自动触发（device_register，device_add，device_create_vargs，device_create） // 向内核添加设备
    （2）将struct device_driver类型的变量注册到内核中时自动触发（driver_register）                                         // 向内核添加驱动
    （3）手动查找同一bus下的所有device_driver，如果有和指定device同名的driver，执行probe操作（device_attach）
    （4）手动查找同一bus下的所有device，如果有和指定driver同名的device，执行probe操作（driver_attach）
    （5）自行调用driver的probe接口，并在该接口中将该driver绑定到某个device结构中----即设置dev->driver（device_bind_driver）

    实际上，probe动作实际是由bus模块实现的，每个bus都有一个drivers_autoprobe变量，用于控制是否在device或者driver注册时，自动probe。该变量默认为1（即自动probe），bus模
块将它开放到sysfs中了，因而可在用户空间修改，进而控制probe行为。