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
typedef   void (*irq_flow_handler_t)(unsigned int irq, struct irq_desc *desc);
/**
 * struct irq_desc - interrupt descriptor
 * @irq_data:       per irq and chip data passed down to chip functions
 * @kstat_irqs:     irq stats per cpu
 * @handle_irq:     highlevel irq-events handler
 * @preflow_handler:    handler called before the flow handler (currently used by sparc)
 * @action:     the irq action chain
 * @status:     status information
 * @core_internal_state__do_not_mess_with_it: core internal status information
 * @depth:      disable-depth, for nested irq_disable() calls
 * @wake_depth:     enable depth, for multiple irq_set_irq_wake() callers
 * @irq_count:      stats field to detect stalled irqs
 * @last_unhandled: aging timer for unhandled count
 * @irqs_unhandled: stats field for spurious unhandled interrupts
 * @threads_handled:    stats field for deferred spurious detection of threaded handlers
 * @threads_handled_last: comparator field for deferred spurious detection of theraded handlers
 * @lock:       locking for SMP
 * @affinity_hint:  hint to user space for preferred irq affinity
 * @affinity_notify:    context for notification of affinity changes
 * @pending_mask:   pending rebalanced interrupts
 * @threads_oneshot:    bitfield to handle shared oneshot threads
 * @threads_active: number of irqaction threads currently running
 * @wait_for_threads:   wait queue for sync_irq to wait for threaded handlers
 * @nr_actions:     number of installed actions on this descriptor
 * @no_suspend_depth:   number of irqactions on a irq descriptor with
 *          IRQF_NO_SUSPEND set
 * @force_resume_depth: number of irqactions on a irq descriptor with
 *          IRQF_FORCE_RESUME set
 * @dir:        /proc/irq/ procfs entry
 * @name:       flow handler name for /proc/interrupts output
 */
struct irq_desc { 
    struct irq_data     irq_data;
    unsigned int __percpu   *kstat_irqs; // IRQ的统计信息
    irq_flow_handler_t  handle_irq; // 见下面的注解
#ifdef CONFIG_IRQ_PREFLOW_FASTEOI
    irq_preflow_handler_t   preflow_handler;
#endif
    struct irqaction    *action;    /* IRQ action list */ // action指向一个struct irqaction的链表。如果一个interrupt request line允许共享，那么该
                                    // 链表中的成员可以是多个，否则，该链表只有一个节点。
    unsigned int        status_use_accessors; // 中断描述符的状态，参考IRQ_xxxx
    unsigned int        core_internal_state__do_not_mess_with_it; // 这个有着很长名字的符号core_internal_state__do_not_mess_with_it在具体使用的
                                    // 时候被被简化成istate，表示internal state。就像这个名字定义的那样，我们最好不要直接修改它。
                                    // #define istate core_internal_state__do_not_mess_with_it
    unsigned int        depth;      /* nested irq disables */ // 我们可以通过enable和disable一个指定的IRQ来控制内核的并发，从而保护临界区的数据。对
                                    // 一个IRQ进行enable和disable的操作可以嵌套（当然一定要成对使用），depth是描述嵌套深度的信息。
    unsigned int        wake_depth; /* nested wake enables */ // wake_depth是和电源管理中的wake up source相关。通过irq_set_irq_wake接口可以enable
                                    // 或者disable一个IRQ中断是否可以把系统从suspend状态唤醒。同样的，对一个IRQ进行wakeup source的enable和disable的
                                    // 操作可以嵌套（当然一定要成对使用），wake_depth是描述嵌套深度的信息。
    unsigned int        irq_count;  /* For detecting broken IRQs */ // irq_count、last_unhandled和irqs_unhandled用于处理broken IRQ 的处理。所谓
                                    // broken IRQ就是由于种种原因（例如错误firmware），IRQ handler没有定向到指定的IRQ上，当一个IRQ没有被处理的时候，
                                    // kernel可以为这个没有被处理的handler启动scan过程，让系统中所有的handler来认领该IRQ。
    unsigned long       last_unhandled; /* Aging timer for unhandled count */
    unsigned int        irqs_unhandled;
    atomic_t        threads_handled;
    int         threads_handled_last;
    raw_spinlock_t      lock;   // 保护该中断描述符的spin lock。
    struct cpumask      *percpu_enabled; // 一个中断描述符可能会有两种情况，一种是该IRQ是global，一旦disable了该irq，那么对于所有的CPU而言都是disable
                                // 的。还有一种情况，就是该IRQ是per CPU的，也就是说，在某个CPU上disable了该irq只是disable了本CPU的IRQ而已，其他的CPU
                                // 仍然是enable的。percpu_enabled是一个描述该IRQ在各个CPU上是否enable的成员。
#ifdef CONFIG_SMP
    const struct cpumask    *affinity_hint; // 和irq affinity相关
    struct irq_affinity_notify *affinity_notify;
#ifdef CONFIG_GENERIC_PENDING_IRQ
    cpumask_var_t       pending_mask;
#endif
#endif
    unsigned long       threads_oneshot; // threads_oneshot、threads_active和wait_for_threads是和IRQ thread相关
    atomic_t        threads_active;
    wait_queue_head_t       wait_for_threads;
#ifdef CONFIG_PM_SLEEP
    unsigned int        nr_actions;
    unsigned int        no_suspend_depth;
    unsigned int        cond_suspend_depth;
    unsigned int        force_resume_depth;
#endif
#ifdef CONFIG_PROC_FS
    struct proc_dir_entry   *dir; // 该IRQ对应的proc接口
#endif
    int         parent_irq;
    struct module       *owner;
    const char      *name;
} ____cacheline_internodealigned_in_smp;
/**
 * struct irq_data - per irq and irq chip data passed down to chip functions
 * @mask:       precomputed bitmask for accessing the chip registers
 * @irq:        interrupt number
 * @hwirq:      hardware interrupt number, local to the interrupt domain
 * @node:       node index useful for balancing
 * @state_use_accessors: status information for irq chip functions.
 *          Use accessor functions to deal with it
 * @chip:       low level interrupt hardware access
 * @domain:     Interrupt translation domain; responsible for mapping
 *          between hwirq number and linux irq number.
 * @parent_data:    pointer to parent struct irq_data to support hierarchy
 *          irq_domain
 * @handler_data:   per-IRQ data for the irq_chip methods
 * @chip_data:      platform-specific per-chip private data for the chip
 *          methods, to allow shared chip implementations
 * @msi_desc:       MSI descriptor
 * @affinity:       IRQ affinity on SMP
 *
 * The fields here need to overlay the ones in irq_desc until we
 * cleaned up the direct references and switched everything over to
 * irq_data.
 */
struct irq_data {
    u32         mask;   // 用于访问芯片寄存器的预计算位掩码
    unsigned int        irq;    // IRQ number
    unsigned long       hwirq;  // HW interrupt ID
    unsigned int        node;   // NUMA node index
    unsigned int        state_use_accessors;    // 底层状态，参考IRQD_xxxx
    struct irq_chip     *chip;  // 该中断描述符对应的irq chip数据结构
    struct irq_domain   *domain;    // 该中断描述符对应的irq domain数据结构
#ifdef  CONFIG_IRQ_DOMAIN_HIERARCHY
    struct irq_data     *parent_data;
#endif
    void            *handler_data;  // 和外设specific handler相关的私有数据
    void            *chip_data; // 和中断控制器相关的私有数据
    struct msi_desc     *msi_desc;
    cpumask_var_t       affinity;   // 和irq affinity相关
};
/**
 * struct irq_chip - hardware interrupt chip descriptor
 *
 * @name:       name for /proc/interrupts
 * @irq_startup:    start up the interrupt (defaults to ->enable if NULL)
 * @irq_shutdown:   shut down the interrupt (defaults to ->disable if NULL)
 * @irq_enable:     enable the interrupt (defaults to chip->unmask if NULL)
 * @irq_disable:    disable the interrupt
 * @irq_ack:        start of a new interrupt
 * @irq_mask:       mask an interrupt source
 * @irq_mask_ack:   ack and mask an interrupt source
 * @irq_unmask:     unmask an interrupt source
 * @irq_eoi:        end of interrupt
 * @irq_set_affinity:   set the CPU affinity on SMP machines
 * @irq_retrigger:  resend an IRQ to the CPU
 * @irq_set_type:   set the flow type (IRQ_TYPE_LEVEL/etc.) of an IRQ
 * @irq_set_wake:   enable/disable power-management wake-on of an IRQ
 * @irq_bus_lock:   function to lock access to slow bus (i2c) chips
 * @irq_bus_sync_unlock:function to sync and unlock slow bus (i2c) chips
 * @irq_cpu_online: configure an interrupt source for a secondary CPU
 * @irq_cpu_offline:    un-configure an interrupt source for a secondary CPU
 * @irq_suspend:    function called from core code on suspend once per chip
 * @irq_resume:     function called from core code on resume once per chip
 * @irq_pm_shutdown:    function called from core code on shutdown once per chip
 * @irq_calc_mask:  Optional function to set irq_data.mask for special cases
 * @irq_print_chip: optional to print special chip info in show_interrupts
 * @irq_request_resources:  optional to request resources before calling
 *              any other callback related to this irq
 * @irq_release_resources:  optional to release resources acquired with
 *              irq_request_resources
 * @irq_compose_msi_msg:    optional to compose message content for MSI
 * @irq_write_msi_msg:  optional to write message content for MSI
 * @irq_get_irqchip_state:  return the internal state of an interrupt
 * @irq_set_irqchip_state:  set the internal state of a interrupt
 * @flags:      chip specific flags
 */
struct irq_chip { // struct irq_chip 包括了若干和具体Interrupt controller相关的callback函数，其被嵌套在struct irq_desc -> struct irq_data之内，
                  // 用于描述对一个 HW interrupt ID 的中断源的操作
    const char  *name;  // 该中断控制器的名字，用于/proc/interrupts中的显示
    unsigned int (*irq_startup)(struct irq_data *data); // start up 对应 HW interrupt ID 的中断。如果不设定的话，会被设定为 enable 函数
    void        (*irq_shutdown)(struct irq_data *data); // shutdown 对应 HW interrupt ID 的中断。如果不设定的话，会被设定为 disable 函数
    void        (*irq_enable)(struct irq_data *data); // 使能对应 HW interrupt ID 的中断。如果不设定的话，会被设定为 unmask 函数
    void        (*irq_disable)(struct irq_data *data); // 失能对应 HW interrupt ID 的中断

    void        (*irq_ack)(struct irq_data *data); // 和具体的硬件相关，有些中断控制器必须在Ack之后（清除pending的状态）才能接受到新的中断。
    void        (*irq_mask)(struct irq_data *data); // 屏蔽指定的的HW interrupt ID
    void        (*irq_mask_ack)(struct irq_data *data); // 屏蔽并ack指定的HW interrupt ID。
    void        (*irq_unmask)(struct irq_data *data); // 取消屏蔽指定的HW interrupt ID
    void        (*irq_eoi)(struct irq_data *data); // 有些中断控制器（例如GIC）提供了这样的寄存器接口，让CPU可以通知中断控制器，它已经处理完一个中断

    int     (*irq_set_affinity)(struct irq_data *data, const struct cpumask *dest, bool force); // 在SMP的情况下，可以通过该callback函数设定CPU affinity
    int     (*irq_retrigger)(struct irq_data *data); // 重新触发一次中断，一般用在中断丢失的场景下。如果硬件不支持retrigger，可以使用软件的方法。
    int     (*irq_set_type)(struct irq_data *data, unsigned int flow_type); // 设定指定的 HW interrupt ID的触发方式，电平触发还是边缘触发
    int     (*irq_set_wake)(struct irq_data *data, unsigned int on); // 和电源管理相关，用来enable/disable指定的中断源作为唤醒的条件。

    void        (*irq_bus_lock)(struct irq_data *data); // 有些interrupt controller是连接到慢速总线上（例如一个i2c接口的IO expander芯片），在
                                                        // 访问这些芯片的时候需要lock住那个慢速bus（只能有一个client在使用I2C bus）
    void        (*irq_bus_sync_unlock)(struct irq_data *data); // 	unlock慢速总线

    void        (*irq_cpu_online)(struct irq_data *data);
    void        (*irq_cpu_offline)(struct irq_data *data);

    void        (*irq_suspend)(struct irq_data *data); // 电源管理相关的callback函数
    void        (*irq_resume)(struct irq_data *data);
    void        (*irq_pm_shutdown)(struct irq_data *data);

    void        (*irq_calc_mask)(struct irq_data *data);

    void        (*irq_print_chip)(struct irq_data *data, struct seq_file *p); // /proc/interrupts中的信息显示
    int     (*irq_request_resources)(struct irq_data *data);
    void        (*irq_release_resources)(struct irq_data *data);

    void        (*irq_compose_msi_msg)(struct irq_data *data, struct msi_msg *msg);
    void        (*irq_write_msi_msg)(struct irq_data *data, struct msi_msg *msg);

    int     (*irq_get_irqchip_state)(struct irq_data *data, enum irqchip_irq_state which, bool *state);
    int     (*irq_set_irqchip_state)(struct irq_data *data, enum irqchip_irq_state which, bool state);

    unsigned long   flags;
};
typedef irqreturn_t (*irq_handler_t)(int, void *);
/**
 * struct irqaction - per interrupt action descriptor
 * @handler:    interrupt handler function
 * @name:   name of the device
 * @dev_id: cookie to identify the device
 * @percpu_dev_id:  cookie to identify the device
 * @next:   pointer to the next irqaction for shared interrupts
 * @irq:    interrupt number
 * @flags:  flags (see IRQF_* above)
 * @thread_fn:  interrupt handler function for threaded interrupts
 * @thread: thread pointer for threaded interrupts
 * @thread_flags:   flags related to @thread
 * @thread_mask:    bitmask for keeping track of @thread activity
 * @dir:    pointer to the proc/irq/NN/name entry
 */
struct irqaction {
    irq_handler_t       handler;
    void            *dev_id;
    void __percpu       *percpu_dev_id;
    struct irqaction    *next;
    irq_handler_t       thread_fn;
    struct task_struct  *thread;
    unsigned int        irq;
    unsigned int        flags;
    unsigned long       thread_flags;
    unsigned long       thread_mask;
    const char      *name;
    struct proc_dir_entry   *dir;
} ____cacheline_internodealigned_in_smp;