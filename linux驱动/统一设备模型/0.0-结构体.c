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
    const char      *init_name; //该设备的名称 /* initial name of the device */ 
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
    bool            offline:1;
};