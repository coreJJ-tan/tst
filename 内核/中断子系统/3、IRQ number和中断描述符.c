1、概念
1.1 什么是中断描述符？
    在linux kernel中，对于每一个外设的IRQ都用struct irq_desc来描述，我们称之中断描述符（struct irq_desc）。linux kernel中会有一，个数据结构
保存了关于所有IRQ的中断描述符信息，我们称之中断描述符DB。当发生中断后，首先获取触发中断的HW interupt ID，然后通过irq domain翻译成IRQ number，
然后通过 IRQ number 就可以获取对应的中断描述符。调用中断描述符中的 highlevel irq-events handler （即 handle_irq() 成员函数） 来进行中断处理
就OK了。而 highlevel irq-events handler 主要进行下面两个操作：
    （1）调用中断描述符的底层irq chip driver进行mask，ack等callback函数，进行interrupt flow control。
    （2）调用该中断描述符上的 action list 中的 specific handler （我们用这个术语来区分具体中断handler和high level的handler）。这个步骤不一
定会执行，这是和中断描述符的当前状态相关，实际上，interrupt flow control是软件（设定一些标志位，软件根据标志位进行处理）和硬件（mask或者unmask 
interrupt controller等）一起控制完成的。

1.2 什么是IRQ number？
    从CPU的角度看，无论外部的Interrupt controller的结构是多么复杂，I do not care，我只关心发生了一个指定外设的中断，需要调用相应的外设中断的
handler就OK了。更准确的说是通用中断处理模块不关心外部interrupt controller的组织细节（电源管理模块当然要关注具体的设备（interrupt controller
也是设备）的拓扑结构）。一言以蔽之，通用中断处理模块可以用一个线性的table来管理一个个的外部中断，这个表的每个元素就是一个irq描述符，在kernel中
定义如下：
struct irq_desc irq_desc[NR_IRQS] __cacheline_aligned_in_smp = {
    [0 ... NR_IRQS-1] = {
        .handle_irq    = handle_bad_irq,
        .depth        = 1,
        .lock        = __RAW_SPIN_LOCK_UNLOCKED(irq_desc->lock),
    }
};
    系统中每一个连接外设的中断线（irq request line）用一个中断描述符来描述，每一个外设的interrupt request line分配一个中断号（irq number），
系统中有多少个中断线（或者叫做中断源）就有多少个中断描述符（struct irq_desc）。NR_IRQS定义了该硬件平台IRQ的最大数目。
    总之，一个静态定义的表格，irq number作为index，每个描述符都是紧密的排在一起，一切看起来很美好，但是现实很残酷的。有些系统可能会定义一个很大
的NR_IRQS，但是只是想用其中的若干个，换句话说，这个静态定义的表格不是每个entry都是有效的，有空洞，如果使用静态定义的表格就会导致了内存很大的浪费。
为什么会有这种需求？我猜是和各个interrupt controller硬件的interrupt ID映射到irq number的算法有关。在这种情况下，静态表格不适合了，我们改用一
个radix tree来保存中断描述符（HW interrupt作为索引）。这时候，每一个中断描述符都是动态分配，然后插入到radix tree中。如果你的系统采用这种策略，
那么需要打开CONFIG_SPARSE_IRQ选项。此外，需要注意的是，在旧内核中，IRQ number和硬件的连接有一定的关系，但是，在引入irq domain后，IRQ number已
经变成一个单纯的number，和硬件没有任何关系。

2、中断的打开和关闭
    开关中断有两种：
    （1）开关local CPU的中断。对于UP，关闭CPU中断就关闭了一切，永远不会被抢占。对于SMP，实际上，没有关全局中断这一说，只能关闭local CPU（代码
运行的那个CPU）
    （2）控制interrupt controller，关闭某个IRQ number对应的中断。更准确的术语是mask或者unmask一个 IRQ。
    本节主要描述的是第一种，也就是控制CPU的中断。当进入high level handler的时候，CPU的中断是关闭的（硬件在进入IRQ processor mode的时候设定的）。
    旧的内核（2.6.35版本之前）认为有两种：slow handler和fast handle。在request irq的时候，对于fast handler，需要传递IRQF_DISABLED的参数，确
保其中断处理过程中是关闭CPU的中断，因为是fast handler，执行很快，即便是关闭CPU中断不会影响系统的性能。但是，并不是每一种外设中断的handler都是那
么快（例如磁盘），因此就有slow handler的概念，说明其在中断处理过程中会耗时比较长。对于这种情况，如果在整个specific handler中关闭CPU中断，对系统
的performance会有影响。因此，对于slow handler，在从high level handler转入specific handler中间会根据IRQF_DISABLED这个flag来决定是否打开中断。
如果没有设定IRQF_DISABLED（slow handler），则打开本CPU的中断。然而，随着软硬件技术的发展：
    （1）硬件方面，CPU越来越快，原来slow handler也可以很快执行完毕
    （2）软件方面，linux kernel提供了更多更好的bottom half的机制
    因此，在新的内核中，比如3.14，IRQF_DISABLED被废弃了。我们可以思考一下，为何要有slow handler？每一个handler不都是应该迅速执行完毕，返回中断
现场吗？此外，任意中断可以打断slow handler执行，从而导致中断嵌套加深，对内核栈也是考验。因此，新的内核中在interrupt specific handler中是全程关
闭CPU中断的。

3、结构体
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
    handle_irq 成员就是highlevel irq-events handler，何谓high level？站在高处自然看不到细节。我认为high level是和specific相对，specific handler
处理具体的事务，例如处理一个按键中断、处理一个磁盘中断。而high level则是对处理各种中断交互过程的一个抽象，根据下列硬件的不同：
    （1）中断控制器
    （2）IRQ trigger type
        highlevel irq-events handler可以分成：
        （a）处理电平触发类型的中断handler（handle_level_irq）
        （b）处理边缘触发类型的中断handler（handle_edge_irq）
        （c）处理简单类型的中断handler（handle_simple_irq）
        （d）处理EOI类型的中断handler（handle_fasteoi_irq）
    会另外有一份文档对high level handler进行更详细的描述。

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
    node 成员用来保存中断描述符的内存位于哪一个memory node上。 对于支持NUMA（Non Uniform Memory Access Architecture）的系统，其内存空间并不是均
一的，而是被划分成不同的node，对于不同的memory node，CPU其访问速度是不一样的。如果一个IRQ大部分（或者固定）由某一个CPU处理，那么在动态分配中断描述
符的时候，应该考虑将内存分配在该CPU访问速度比较快的memory node上。

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

4、初始化相关的中断描述符的API
4.1 静态定义的中断描述符初始化
