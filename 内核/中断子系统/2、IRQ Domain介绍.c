http://www.wowotech.net/irq_subsystem/irq-domain.html

1、概述
    在linux kernel中，我们使用下面两个ID来标识一个来自外设的中断：
(1)IRQ number
    CPU需要为每一个外设中断编号，我们称之IRQ Number。这个IRQ number是一个虚拟的interrupt ID，和硬件无关，仅仅是被CPU用来标识一个外设中断。
(2)HW interrupt ID
    对于中断控制器而言，它收集了多个外设的interrupt request line并向上传递，因此，中断控制器需要对外设中断源进行编码。中断控制器用HW interrupt ID
来标识外设的中断。在中断控制器级联的情况下，仅仅用HW interrupt ID已经不能唯一标识一个外设中断，还需要知道该HW interrupt ID所属的中断控制器
（HW interrupt ID在不同的中断控制器上是会重复编码的）。
    这样，CPU和中断控制器在标识中断上就有了一些不同的概念，但是，对于驱动工程师而言，我们和CPU视角是一样的，我们只希望得到一个IRQ number，而不关系具
体是那个中断控制器上的那个HW interrupt ID。这样一个好处是在中断相关的硬件发生变化的时候，驱动软件不需要修改。因此，linux kernel中的中断子系统需要提
供一个将HW interrupt ID映射到IRQ number上来的机制，这就是本文主要的内容。

2、相关接口
2.1 相关结构体
/**
 * struct irq_domain_ops - Methods for irq_domain objects
 * @match: Match an interrupt controller device node to a host, returns
 *         1 on a match
 * @map: Create or update a mapping between a virtual irq number and a hw
 *       irq number. This is called only once for a given mapping.
 * @unmap: Dispose of such a mapping
 * @xlate: Given a device tree node and interrupt specifier, decode
 *         the hardware irq number and linux irq type value.
 *
 * Functions below are provided by the driver and called whenever a new mapping
 * is created or an old mapping is disposed. The driver can then proceed to
 * whatever internal data structures management is required. It also needs
 * to setup the irq_desc when returning from map().
 */
struct irq_domain_ops {
    int (*match)(struct irq_domain *d, struct device_node *node); // 用于匹配domain
    int (*map)(struct irq_domain *d, unsigned int virq, irq_hw_number_t hw); // 线性映射
    void (*unmap)(struct irq_domain *d, unsigned int virq); 
    int (*xlate)(struct irq_domain *d, struct device_node *node, const u32 *intspec, unsigned int intsize, unsigned long *out_hwirq, 
                unsigned int *out_type); // 通过参数,线性hirq获取

#ifdef  CONFIG_IRQ_DOMAIN_HIERARCHY
    /* extended V2 interfaces to support hierarchy irq_domains */
    int (*alloc)(struct irq_domain *d, unsigned int virq, unsigned int nr_irqs, void *arg); //树形节点申请,映射（只有树形才有这个）
    void (*free)(struct irq_domain *d, unsigned int virq, unsigned int nr_irqs);
    void (*activate)(struct irq_domain *d, struct irq_data *irq_data); // 中断激活
    void (*deactivate)(struct irq_domain *d, struct irq_data *irq_data);
#endif
};
    xlate函数：语义是翻译（translate）的意思，那么到底翻译什么呢？在DTS文件中，各个使用中断的device node会通过一些属性（例如interrupts和interrupt-parent
属性）来提供中断信息给kernel以便kernel可以正确的进行driver的初始化动作。这里，interrupts属性所表示的interrupt specifier只能由具体的interrupt controller
（也就是irq domain）来解析。而xlate函数就是将指定的设备（node参数）上若干个（intsize参数）中断属性（intspec参数）翻译成HW interrupt ID（out_hwirq参数）和
trigger类型（out_type）。
    match函数：是判断一个指定的中断控制器（node参数）是否和一个irq domain匹配（d参数），如果匹配的话，返回1。实际上，内核中很少定义这个callback函数，实际
上struct irq_domain中有一个of_node指向了对应的中断控制器的device node，因此，如果不提供该函数，那么默认的匹配函数其实就是判断irq domain的of_node成员是否
等于传入的node参数。
    map和unmap是操作相反的函数，我们描述其中之一就OK了。调用map函数的时机是在创建（或者更新）HW interrupt ID（hw参数）和IRQ number（virq参数）关系的时候。
其实，从发生一个中断到调用该中断的handler仅仅调用一个request_threaded_irq是不够的，还需要针对该irq number设定：
    （1）设定该IRQ number对应的中断描述符（struct irq_desc）的irq chip
    （2）设定该IRQ number对应的中断描述符的highlevel irq-events handler
    （3）设定该IRQ number对应的中断描述符的 irq chip data

这些设定不适合由具体的硬件驱动来设定，因此在中断控制器，也就是irq domain的callback函数中设定。

/**
 * struct irq_domain - Hardware interrupt number translation object
 * @link: Element in global irq_domain list.
 * @name: Name of interrupt domain
 * @ops: pointer to irq_domain methods
 * @host_data: private data pointer for use by owner.  Not touched by irq_domain
 *             core code.
 * @flags: host per irq_domain flags
 *
 * Optional elements
 * @of_node: Pointer to device tree nodes associated with the irq_domain. Used
 *           when decoding device tree interrupt specifiers.
 * @gc: Pointer to a list of generic chips. There is a helper function for
 *      setting up one or more generic chips for interrupt controllers
 *      drivers using the generic chip library which uses this pointer.
 * @parent: Pointer to parent irq_domain to support hierarchy irq_domains
 *
 * Revmap data, used internally by irq_domain
 * @revmap_direct_max_irq: The largest hwirq that can be set for controllers that
 *                         support direct mapping
 * @revmap_size: Size of the linear map table @linear_revmap[]
 * @revmap_tree: Radix map tree for hwirqs that don't fit in the linear map
 * @linear_revmap: Linear table of hwirq->virq reverse mappings
 */
struct irq_domain { // 一个中断控制器就可以看成是一个domain
    struct list_head link;  // 连接各个domain的链表节点，链表头为 irq_domain_list，是个全局链表，定义在 ./kernel/irq/irqdomain.c 中
    const char *name;   // domain的名字
    const struct irq_domain_ops *ops; // 当前domain的处理函数
    void *host_data; // 定义了底层中断控制器使用的私有数据，和具体的中断控制器相关（对于GIC，该指针指向一个struct gic_chip_data数据结构）。
    unsigned int flags;

    /* Optional data */
    struct device_node *of_node; // 该domain对应的中断控制器的device node
    struct irq_domain_chip_generic *gc;
#ifdef  CONFIG_IRQ_DOMAIN_HIERARCHY
    struct irq_domain *parent; // 树形结构的父节点domain
#endif

    /* reverse map data. The linear map gets appended to the irq_domain */
    irq_hw_number_t hwirq_max; // 该domain中最大的那个HW interrupt ID
    unsigned int revmap_direct_max_irq; // hwirq和virq 1:1映射支持的最大数量
    unsigned int revmap_size; // 线性映射的size，对于 Radix Tree map和no map，该值等于0
    
    struct radix_tree_root revmap_tree; // Radix Tree map使用到的radix tree root node
    unsigned int linear_revmap[]; // 线性映射使用的查找表  这是个柔性数组
};

    对于线性映射：
    （1） linear_revmap 保存了一个线性的查找表，index是HW interrupt ID，table中保存了IRQ number值
    （2） revmap_size 等于线性的查找表的size。
    （3） hwirq_max 保存了最大的HW interrupt ID
    （4） revmap_direct_max_irq 没有用，设定为0。revmap_tree没有用。

    对于Radix Tree map：
    （1） linear_revmap 没有用，revmap_size等于0。
    （2） hwirq_max 没有用，设定为一个最大值。
    （3） revmap_direct_max_irq 没有用，设定为0。
    （4） revmap_tree 指向Radix tree的root node。

2.2 向系统注册irq domain
    当早期的系统只存在一个中断控制器的时候，而且中断数目也不多的时候，一个很简单的做法就是一个中断号对应到中断控制器的一个号，可以说是简单的线性映射。
而当一个系统中有多个中断控制器的时候，而且中断号也逐渐增加。内核为了应对此问题，引入了IRQ-domain的概念。
    通用中断处理模块中有一个irq domain的子模块，该模块将HW interrupt ID到IRQ number的映射关系分成了三类：
    （1）线性映射。其实就是一个查找表，HW interrupt ID作为index，通过查表可以获取对应的IRQ number。对于线性映射而言，中断控制器对其HW interrupt ID
进行编码的时候要满足一定的条件：hw ID不能过大，而且ID排列最好是紧密的。对于线性映射，其接口API如下：
/**
 * irq_domain_add_linear() - Allocate and register a linear revmap irq_domain.
 * @of_node: pointer to interrupt controller's device tree node.
 * @size: Number of interrupts in the domain.
 * @ops: map/unmap domain callbacks
 * @host_data: Controller private data pointer
 */
static inline struct irq_domain *irq_domain_add_linear(struct device_node *of_node, unsigned int size, const struct irq_domain_ops *ops, void *host_data)

    （2）Radix Tree map。建立一个Radix Tree来维护HW interrupt ID到IRQ number映射关系。HW interrupt ID作为lookup key，在Radix Tree检索到IRQ number。
如果的确不能满足线性映射的条件，可以考虑Radix Tree map。实际上，内核中使用Radix Tree map的只有powerPC和MIPS的硬件平台。对于Radix Tree map，其接口API如下：
static inline struct irq_domain *irq_domain_add_tree(struct device_node *of_node, const struct irq_domain_ops *ops, void *host_data)

    （3）no map。有些中断控制器很强，可以通过寄存器配置HW interrupt ID而不是由物理连接决定的。例如PowerPC 系统使用的MPIC (Multi-Processor Interrupt Controller)。
在这种情况下，不需要进行映射，我们直接把IRQ number写入HW interrupt ID配置寄存器就OK了，这时候，生成的HW interrupt ID就是IRQ number，也就不需要进行mapping了。
对于这种类型的映射，其接口API如下：
static inline struct irq_domain *irq_domain_add_nomap(struct device_node *of_node, unsigned int max_irq, const struct irq_domain_ops *ops, void *host_data)

    这上面三种方式主要是向系统注册一个irq domain，具体HW interrupt ID和IRQ number的映射关系都是空的，因此，具体各个irq domain如何管理映射所需要的
database还是需要建立的。

2.3 为irq domain创建映射
    对于线性映射的irq domain，我们需要建立线性映射的查找表，对于Radix Tree map，我们要把那个反应IRQ number和HW interrupt ID的Radix tree建立起来。
创建映射有四个接口函数：
    （1）调用irq_create_mapping函数建立HW interrupt ID和IRQ number的映射关系。该接口函数以irq domain和HW interrupt ID为参数，返回IRQ number（这
个IRQ number是动态分配的）。该函数的原型定义如下：
/**
 * irq_create_mapping() - Map a hardware interrupt into linux irq space
 * @domain: domain owning this hardware interrupt or NULL for default domain
 * @hwirq: hardware irq number in that domain space
 *
 * Only one mapping per hardware interrupt is permitted. Returns a linux irq number.
 * If the sense/trigger is to be specified, set_irq_type() should be called on the number returned from that call.
 */
unsigned int irq_create_mapping(struct irq_domain *domain, irq_hw_number_t hwirq)
{
    int virq;
    pr_debug("irq_create_mapping(0x%p, 0x%lx)\n", domain, hwirq);
    if (domain == NULL) 
        domain = irq_default_domain;
    if (domain == NULL)
        return 0; // WARN(1, "%s(, %lx) called with NULL domain\n", __func__, hwirq);
    pr_debug("-> using domain @%p\n", domain);
    virq = irq_find_mapping(domain, hwirq); /* Check if mapping already exists */
    if (virq)
        return virq; // pr_debug("-> existing mapping on virq %d\n", virq);
    virq = irq_domain_alloc_descs(-1, 1, hwirq, of_node_to_nid(domain->of_node)); /* Allocate a virtual interrupt number */
    if (virq <= 0)
        return 0; // pr_debug("-> virq allocation failed\n");
    if (irq_domain_associate(domain, virq, hwirq))
        irq_free_desc(virq);
        return 0;
    pr_debug("irq %lu on domain %s mapped to virtual irq %u\n", hwirq, of_node_full_name(domain->of_node), virq);
    return virq;
}
    驱动调用该函数的时候必须提供HW interrupt ID，也就是意味着driver知道自己使用的HW interrupt ID，而一般情况下，HW interrupt ID其实对具体的driver
应该是不可见的，不过有些场景比较特殊，例如GPIO类型的中断，它的HW interrupt ID和GPIO有着特定的关系，driver知道自己使用那个GPIO，也就是知道使用哪一个HW interrupt ID了。

    （2）irq_create_strict_mappings。这个接口函数用来为一组HW interrupt ID建立映射。具体函数的原型定义如下：
/**
 * irq_create_strict_mappings() - Map a range of hw irqs to fixed linux irqs
 * @domain: domain owning the interrupt range
 * @irq_base: beginning of linux IRQ range
 * @hwirq_base: beginning of hardware IRQ range
 * @count: Number of interrupts to map
 *
 * This routine is used for allocating and mapping a range of hardware
 * irqs to linux irqs where the linux irq numbers are at pre-defined
 * locations. For use by controllers that already have static mappings
 * to insert in to the domain.
 *
 * Non-linear users can use irq_create_identity_mapping() for IRQ-at-a-time
 * domain insertion.
 *
 * 0 is returned upon success, while any failure to establish a static
 * mapping is treated as an error.
 */
int irq_create_strict_mappings(struct irq_domain *domain, unsigned int irq_base, irq_hw_number_t hwirq_base, int count)
{
    int ret;
    ret = irq_alloc_descs(irq_base, irq_base, count, of_node_to_nid(domain->of_node));
    if (unlikely(ret < 0))
        return ret;
    irq_domain_associate_many(domain, irq_base, hwirq_base, count);
    return 0;
}

    （3）irq_create_of_mapping。利用device tree进行映射关系的建立。具体函数的原型定义如下：
unsigned int irq_create_of_mapping(struct of_phandle_args *irq_data)
{
    struct irq_domain *domain;
    irq_hw_number_t hwirq;
    unsigned int type = IRQ_TYPE_NONE;
    int virq;

    domain = irq_data->np ? irq_find_host(irq_data->np) : irq_default_domain;
    if (!domain) {
        pr_warn("no irq domain found for %s !\n",
            of_node_full_name(irq_data->np));
        return 0;
    }

    /* If domain has no translation, then we assume interrupt line */
    if (domain->ops->xlate == NULL)
        hwirq = irq_data->args[0];
    else {
        if (domain->ops->xlate(domain, irq_data->np, irq_data->args,
                    irq_data->args_count, &hwirq, &type))
            return 0;
    }

    if (irq_domain_is_hierarchy(domain)) {
        /*
         * If we've already configured this interrupt,
         * don't do it again, or hell will break loose.
         */
        virq = irq_find_mapping(domain, hwirq);
        if (virq)
            return virq;

        virq = irq_domain_alloc_irqs(domain, 1, NUMA_NO_NODE, irq_data);
        if (virq <= 0)
            return 0;
    } else {
        /* Create mapping */
        virq = irq_create_mapping(domain, hwirq);
        if (!virq)
            return virq;
    }

    /* Set type if specified and different than the current one */
    if (type != IRQ_TYPE_NONE &&
        type != irq_get_trigger_type(virq))
        irq_set_irq_type(virq, type);
    return virq;
}
    通常，一个普通设备的device tree node已经描述了足够的中断信息，在这种情况下，该设备的驱动在初始化的时候可以调用irq_of_parse_and_map这个接口函数进
行该device node中和中断相关的内容（interrupts和interrupt-parent属性）进行分析，并建立映射关系，具体代码如下：
/**
 * irq_of_parse_and_map - Parse and map an interrupt into linux virq space
 * @dev: Device node of the device whose interrupt is to be mapped
 * @index: Index of the interrupt to map
 *
 * This function is a wrapper that chains of_irq_parse_one() and
 * irq_create_of_mapping() to make things easier to callers
 */
unsigned int irq_of_parse_and_map(struct device_node *dev, int index)
{
    struct of_phandle_args oirq;
    if (of_irq_parse_one(dev, index, &oirq)) // 分析device node中的interrupt相关属性
        return 0;
    return irq_create_of_mapping(&oirq); // 创建映射，并返回对应的IRQ number
}
    对于一个使用Device tree的普通驱动程序（我们推荐这样做），基本上初始化需要调用irq_of_parse_and_map获取IRQ number，然后调用request_threaded_irq
申请中断handler。

    （4）irq_create_direct_mapping。这是给no map那种类型的interrupt controller使用的
unsigned int irq_create_direct_mapping(struct irq_domain *domain)；

3、中断相关的Device Tree
    对于那些产生中断的外设，我们需要定义interrupt-parent和interrupts属性：
    （1）interrupt-parent。表明该外设的interrupt request line物理的连接到了哪一个中断控制器上
    （2）interrupts。这个属性描述了具体该外设产生的interrupt的细节信息（也就是传说中的interrupt specifier）。例如：HW interrupt ID（由该外设的device node
中的interrupt-parent指向的interrupt controller解析）、interrupt触发类型等。
    
    对于Interrupt controller，我们需要定义interrupt-controller和#interrupt-cells的属性：
    （1）interrupt-controller。表明该device node就是一个中断控制器
    （2）#interrupt-cells。该中断控制器用多少个cell（一个cell就是一个32-bit的单元）描述一个外设的interrupt request line。？具体每个cell表示什么样的含义由
interrupt controller自己定义。
    （3）interrupts和interrupt-parent。对于那些不是root 的interrupt controller，其本身也是作为一个产生中断的外设连接到其他的interrupt controller上，因此
也需要定义interrupts和interrupt-parent的属性。

4、映射关系的建立 ---- Mapping DB的建立
4.1 概述
    系统中HW interrupt ID和IRQ number的mapping DB是在整个系统初始化的过程中建立起来的，过程如下：
    （1）DTS文件描述了系统中的interrupt controller以及外设IRQ的拓扑结构，在linux kernel启动的时候，由bootloader传递给kernel（实际传递的是DTB）。
    （2）在Device Tree初始化的时候，形成了系统内所有的device node的树状结构，当然其中包括所有和中断拓扑相关的数据结构（所有的interrupt controller的node和使
用中断的外设node）
    （3）在machine driver初始化的时候会调用of_irq_init函数，在该函数中会扫描所有interrupt controller的节点，并调用适合的interrupt controller driver进行初
始化。毫无疑问，初始化需要注意顺序，首先初始化root，然后first level，second level，最好是leaf node。在初始化的过程中，一般会调用上节中的接口函数向系统增加
irq domain。有些interrupt controller会在其driver初始化的过程中创建映射
    （4）在各个driver初始化的过程中，创建映射

4.2 中断控制器初始化的过程中，注册irq domain
    以GIC的代码为例。具体代码在gic_of_init->gic_init_bases中：
    void __init gic_init_bases(unsigned int gic_nr, int irq_start, void __iomem *dist_base, void __iomem *cpu_base, u32 percpu_offset, struct device_node *node)

4.3 在各个硬件外设的驱动初始化过程中，创建HW interrupt ID和IRQ number的映射关系
    上面的描述过程中，已经提及：设备的驱动在初始化的时候可以调用irq_of_parse_and_map()这个接口函数进行该device node中和中断相关的内容（interrupts和interrupt-parent
属性）进行分析，并建立映射关系。深挖到内部，irq_of_parse_and_map() 这个函数最终会调用上面提到的 irq_create_mapping()来创建HW interrupt ID和IRQ number的映射关系。

5、将HW interrupt ID转成IRQ number
    创建了庞大的HW interrupt ID到IRQ number的mapping DB，最终还是要使用。具体的使用场景是在CPU相关的处理函数中，程序会读取硬件interrupt ID，并转成IRQ number，
调用对应的irq event handler。