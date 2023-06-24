#ifdef CONFIG_SPARSE_IRQ    // 通过 radix tree 的方式映射中断号
struct irq_desc *irq_to_desc(unsigned int irq) // 通过 IRQ number 获取对应的中断描述符结构体的指针
{
    return radix_tree_lookup(&irq_desc_tree, irq);
}
int nr_irqs = NR_IRQS; // 这是个全局变量，保存着系统中中断描述符的个数，也可认为nr_irqs是当前系统中IRQ number的最大值
int __init early_irq_init(void) // 该函数在 kernel_start 中被调用
{
    int i, initcnt, node = first_online_node;
    struct irq_desc *desc;
    init_irq_default_affinity();    // 初始化默认的smp irq affinity机制，非 SMP 系统这是个空函数
        alloc_cpumask_var(&irq_default_affinity, GFP_NOWAIT);
        cpumask_setall(irq_default_affinity);
    /* Let arch update nr_irqs and return the nr of preallocated irqs */
    initcnt = arch_probe_nr_irqs(); // 体系结构相关的代码来决定预先分配的中断描述符的个数
    printk(KERN_INFO "NR_IRQS:%d nr_irqs:%d %d\n", NR_IRQS, nr_irqs, initcnt);
    if (WARN_ON(nr_irqs > IRQ_BITMAP_BITS))
        nr_irqs = IRQ_BITMAP_BITS;
    if (WARN_ON(initcnt > IRQ_BITMAP_BITS))
        initcnt = IRQ_BITMAP_BITS;
    if (initcnt > nr_irqs)
        nr_irqs = initcnt;
    for (i = 0; i < initcnt; i++)
        desc = alloc_desc(i, node, NULL); // 申请动态描述符并初始化，初始化的内容和下面静态分配后初始化的一样
        set_bit(i, allocated_irqs); // allocated_irqs是个全局变量，可看成一段连续的内存空间，该空间占用 bits 数为 IRQ_BITMAP_BITS
                                    // 每一bit是一个中断描述符已经申请过内存的标志，这里就是在alloc之后设置对应的标志位, 标识该IRQ已经分配
        irq_insert_desc(i, desc); // 向 irq_desc_tree （这是个 radix tree）中插入 desc 节点
    return arch_early_irq_init(); // 调用架构相关的初始化函数
}
#else /* !CONFIG_SPARSE_IRQ */ // 使用线性方式映射中断号
struct irq_desc irq_desc[NR_IRQS] __cacheline_aligned_in_smp = {    // 中断描述符lookup table
    [0 ... NR_IRQS-1] = {
        .handle_irq = handle_bad_irq,
        .depth      = 1,
        .lock       = __RAW_SPIN_LOCK_UNLOCKED(irq_desc->lock),
    }
};
int __init early_irq_init(void)
{
    int count, i, node = first_online_node;
    struct irq_desc *desc;
    init_irq_default_affinity(); // 同上
    printk(KERN_INFO "NR_IRQS:%d\n", NR_IRQS);
    desc = irq_desc;
    count = ARRAY_SIZE(irq_desc);
    for (i = 0; i < count; i++) // 遍历整个lookup table，对每一个entry进行初始化
        desc[i].kstat_irqs = alloc_percpu(unsigned int); // 分配per cpu的irq统计信息需要的内存
        alloc_masks(&desc[i], GFP_KERNEL, node); // 分配中断描述符中需要的cpu mask内存
        raw_spin_lock_init(&desc[i].lock);
        lockdep_set_class(&desc[i].lock, &irq_desc_lock_class);
        desc_set_defaults(i, &desc[i], node, NULL);
    return arch_early_irq_init(); // 调用架构相关的初始化函数
}
struct irq_desc *irq_to_desc(unsigned int irq) // 通过 IRQ number 获取对应的中断描述符结构体的指针
{
    return (irq < NR_IRQS) ? irq_desc + irq : NULL;
}
#endif /* END CONFIG_SPARSE_IRQ */

#define irq_alloc_descs(irq, from, cnt, node)   \
    __irq_alloc_descs(irq, from, cnt, node, THIS_MODULE)
/**
 * irq_alloc_descs - allocate and initialize a range of irq descriptors
 * @irq:    Allocate for specific irq number if irq >= 0
 * @from:   Start the search from this irq number
 * @cnt:    Number of consecutive irqs to allocate.
 * @node:   Preferred node on which the irq descriptor should be allocated
 * @owner:  Owning module (can be NULL)
 *
 * Returns the first irq number or error code
 */
int __ref __irq_alloc_descs(int irq, unsigned int from, unsigned int cnt, int node, struct module *owner)
{
    int start, ret;
    if (!cnt)
        return -EINVAL;
    if (irq >= 0) 
        if (from > irq)
            return -EINVAL;
        from = irq;
    else 
        /*
         * For interrupts which are freely allocated the
         * architecture can force a lower bound to the @from
         * argument. x86 uses this to exclude the GSI space.
         */
        from = arch_dynirq_lower_bound(from);
    mutex_lock(&sparse_irq_lock);
    start = bitmap_find_next_zero_area(allocated_irqs, IRQ_BITMAP_BITS, from, cnt, 0);
    ret = -EEXIST;
    if (irq >=0 && start != irq)
        goto err;
    if (start + cnt > nr_irqs)
        ret = irq_expand_nr_irqs(start + cnt);
        if (ret)
            goto err;
    bitmap_set(allocated_irqs, start, cnt);
    mutex_unlock(&sparse_irq_lock);
    return alloc_descs(start, cnt, node, owner);
err:
    mutex_unlock(&sparse_irq_lock);
    return ret;
}
/**
 * irq_free_descs - free irq descriptors
 * @from:   Start of descriptor range
 * @cnt:    Number of consecutive irqs to free
 */
void irq_free_descs(unsigned int from, unsigned int cnt)
{
    int i;

    if (from >= nr_irqs || (from + cnt) > nr_irqs)
        return;

    for (i = 0; i < cnt; i++)
        free_desc(from + i);

    mutex_lock(&sparse_irq_lock);
    bitmap_clear(allocated_irqs, from, cnt);
    mutex_unlock(&sparse_irq_lock);
}
/**
 * generic_handle_irq - Invoke the handler for a particular irq
 * @irq:    The irq number to handle
 *
 */
int generic_handle_irq(unsigned int irq)
{
    struct irq_desc *desc = irq_to_desc(irq);

    if (!desc)
        return -EINVAL;
    generic_handle_irq_desc(irq, desc);
    return 0;
}
static inline struct irq_data *irq_desc_get_irq_data(struct irq_desc *desc)
{
    return &desc->irq_data;
}
static inline struct irq_chip *irq_desc_get_chip(struct irq_desc *desc)
{
    return desc->irq_data.chip;
}
static inline void *irq_desc_get_chip_data(struct irq_desc *desc)
{
    return desc->irq_data.chip_data;
}
static inline void *irq_desc_get_handler_data(struct irq_desc *desc)
{
    return desc->irq_data.handler_data;
}
static inline struct msi_desc *irq_desc_get_msi_desc(struct irq_desc *desc)
{
    return desc->irq_data.msi_desc;
}
/*
 * Architectures call this to let the generic IRQ layer
 * handle an interrupt. If the descriptor is attached to an
 * irqchip-style controller then we call the ->handle_irq() handler,
 * and it calls __do_IRQ() if it's attached to an irqtype-style controller.
 */
static inline void generic_handle_irq_desc(unsigned int irq, struct irq_desc *desc)
{
    desc->handle_irq(irq, desc);
}
#ifdef CONFIG_HANDLE_DOMAIN_IRQ
/*
 * Convert a HW interrupt number to a logical one using a IRQ domain,
 * and handle the result interrupt number. Return -EINVAL if
 * conversion failed. Providing a NULL domain indicates that the
 * conversion has already been done.
 */
static inline int handle_domain_irq(struct irq_domain *domain, unsigned int hwirq, struct pt_regs *regs)
{
    return __handle_domain_irq(domain, hwirq, true, regs);
}
#endif
