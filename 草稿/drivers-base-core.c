struct device {
    struct device       *parent;
    struct device_private   *p;
    struct kobject kobj;
    const char      *init_name; /* initial name of the device */
    const struct device_type *type;
    struct mutex        mutex;  /* mutex to synchronize calls to its driver. */
    struct bus_type *bus;       /* type of bus device is on */
    struct device_driver *driver;   /* which driver has allocated this device */
    void        *platform_data; /* Platform specific data, device core doesn't touch it */
    void        *driver_data;   /* Driver data, set and get with dev_set/get_drvdata */
    struct dev_pm_info  power;
    struct dev_pm_domain    *pm_domain;
    #ifdef CONFIG_PINCTRL
        struct dev_pin_info *pins;
    #endif
    #ifdef CONFIG_NUMA
        int     numa_node;  /* NUMA node this device is close to */
    #endif
    u64     *dma_mask;  /* dma mask (if dma'able device) */
    u64     coherent_dma_mask;/* Like dma_mask, but for alloc_coherent mappings as not all hardware supports 64 bit addresses for consistent allocations such descriptors. */
    unsigned long   dma_pfn_offset;
    struct device_dma_parameters *dma_parms;
    struct list_head    dma_pools;  /* dma pools (if dma'ble) */
    struct dma_coherent_mem *dma_mem; /* internal for coherent mem override */
    #ifdef CONFIG_DMA_CMA
        struct cma *cma_area;       /* contiguous memory area for dma allocations */
    #endif
    /* arch specific additions */
    struct dev_archdata archdata;
    struct device_node  *of_node; /* associated device tree node */
    struct fwnode_handle    *fwnode; /* firmware device node */
    dev_t           devt;   /* dev_t, creates the sysfs "dev" */
    u32         id; /* device instance */
    spinlock_t      devres_lock;
    struct list_head    devres_head;
    struct klist_node   knode_class;
    struct class        *class;
    const struct attribute_group **groups;  /* optional groups */
    void    (*release)(struct device *dev);
    struct iommu_group  *iommu_group;
    bool            offline_disabled:1;
    bool            offline:1;
};



void device_initialize(struct device *dev)
    dev->kobj.kset = devices_kset;  // devices_kset是一个全局变量，是在devices_init() 函数中被赋值的
    kobject_init(&dev->kobj, &device_ktype);
    INIT_LIST_HEAD(&dev->dma_pools);
    mutex_init(&dev->mutex);
    lockdep_set_novalidate_class(&dev->mutex);
    spin_lock_init(&dev->devres_lock);
    INIT_LIST_HEAD(&dev->devres_head);
    device_pm_init(dev);
    set_dev_node(dev, -1);


static const struct kset_uevent_ops device_uevent_ops = {
    .filter =   dev_uevent_filter,
    .name =     dev_uevent_name,
    .uevent =   dev_uevent,
};


int __init devices_init(void)
    devices_kset = kset_create_and_add("devices", &device_uevent_ops, NULL); // devices_kset是一个全局变量,在此赋值
    if (!devices_kset)
        return -ENOMEM;
    dev_kobj = kobject_create_and_add("dev", NULL);
    if (!dev_kobj)
        goto dev_kobj_err;
    sysfs_dev_block_kobj = kobject_create_and_add("block", dev_kobj);
    if (!sysfs_dev_block_kobj)
        goto block_kobj_err;
    sysfs_dev_char_kobj = kobject_create_and_add("char", dev_kobj);
    if (!sysfs_dev_char_kobj)
        goto char_kobj_err;
    return 0;
    char_kobj_err:
        kobject_put(sysfs_dev_block_kobj);
    block_kobj_err:
        kobject_put(dev_kobj);
    dev_kobj_err:
        kset_unregister(devices_kset);
        return -ENOMEM;
