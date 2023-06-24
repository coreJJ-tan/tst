1、概念
1.1 什么是中断描述符？
    在linux kernel中，对于每一个外设的IRQ都用struct irq_desc来描述，我们称之中断描述符（struct irq_desc）。linux kernel中会有一个数据结构
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
struct irq_desc irq_desc[NR_IRQS] __cacheline_aligned_in_smp = { // 中断描述符lookup table
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
---- struct irq_desc
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

---- struct irqaction
---- struct irq_data
    node 成员用来保存中断描述符的内存位于哪一个memory node上。 对于支持NUMA（Non Uniform Memory Access Architecture）的系统，其内存空间并不是均
一的，而是被划分成不同的node，对于不同的memory node，CPU其访问速度是不一样的。如果一个IRQ大部分（或者固定）由某一个CPU处理，那么在动态分配中断描述
符的时候，应该考虑将内存分配在该CPU访问速度比较快的memory node上。

---- struct irq_chip

4、初始化相关的中断描述符的API
4.1 静态定义的中断描述符初始化
---- int __init early_irq_init(void)
    该函数在kernel_start函数中被调用一次
    主要实现的功能有：
        遍历整个中断描述符的lookup table，对每一个entry进行初始化，这个lookup table在内核中是静态定义的，如下：
        struct irq_desc irq_desc[NR_IRQS] __cacheline_aligned_in_smp = { ... }
        具体初始化的内容为：申请各种内存，初始化spin lock，设置一些默认值等

4.2 使用Radix tree的中断描述符初始化
---- int __init early_irq_init(void)
    和4.1所定义的函数名一样，具体通过 CONFIG_SPARSE_IRQ 宏区分，该函数在kernel_start函数中被调用一次
    主要实现的功能有：
        初始化默认的smp irq affinity机制
        初始化 nr_irqs 的值，nr_irqs（是个全局变量）是当前系统中IRQ number的最大值
        申请initcnt数量的中断描述符（struct irq_desc），并插入对应的radix tree中（initcnt是当前CPU体系结构中支持的中断描述符的个数）
    说明：即便是配置了CONFIG_SPARSE_IRQ选项，在中断描述符初始化的时候，也有机会预先分配一定数量（initcnt数量）的IRQ。这个数量由arch_probe_nr_irqs决定

4.3 分配和释放中断描述符
    对于使用Radix tree来保存中断描述符DB的linux kernel，其中断描述符是动态分配的，可以使用irq_alloc_descs()和irq_free_descs()来分配和释放中断描述符。
alloc_desc函数也会对中断描述符进行初始化，初始化的内容和静态定义的中断描述符初始化过程是一样的。最大可以分配的ID是IRQ_BITMAP_BITS，定义如下：
#ifdef CONFIG_SPARSE_IRQ
# define IRQ_BITMAP_BITS    (NR_IRQS + 8196)----对于Radix tree，除了预分配的，还可以动态分配8196个中断描述符
#else                                                                             
# define IRQ_BITMAP_BITS    NR_IRQS----对于静态定义的，IRQ最大值就是NR_IRQS
#endif

5、和中断控制器相关的中断描述符的接口
    这部分的接口主要有两类，irq_desc_get_xxx和irq_set_xxx，由于get接口API非常简单，这里不再描述，主要描述set类别的接口API。此外，还有一些locked版
本的set接口API，定义为__irq_set_xxx，这些API的调用者应该已经持有保护irq desc的spinlock，因此，这些locked版本的接口没有中断描述符的spin lock进行操
作。这些接口有自己特定的使用场合，这里也不详细描述了.

---- int irq_set_chip(unsigned int irq, struct irq_chip *chip)
    这个接口函数用来设定中断描述符中desc->irq_data.chip成员

---- int irq_set_irq_type(unsigned int irq, unsigned int type)
    这个函数是用来设定该irq number的trigger type的。
    参见上面这两个函数的内部代码，有些疑问，第一个问题就是：为何irq_set_chip接口函数使用irq_get_desc_lock来获取中断描述符，而irq_set_irq_type这个
函数却需要irq_get_desc_buslock呢？其实也很简单，irq_set_chip不需要访问底层的irq chip（也就是interrupt controller），但是irq_set_irq_type需要。
设定一个IRQ的trigger type最终要调用desc->irq_data.chip->irq_set_type函数对底层的interrupt controller进行设定。这时候，问题来了，对于嵌入SOC内部
的interrupt controller，当然没有问题，因为访问这些中断控制器的寄存器memory map到了CPU的地址空间，访问非常的快，因此，关闭中断＋spin lock来保护中断
描述符当然没有问题，但是，如果该interrupt controller是一个I2C接口的IO expander芯片（这类芯片是扩展的IO，也可以提供中断功能），这时，让其他CPU进行
spin操作太浪费CPU时间了（bus操作太慢了，会spin很久的）。肿么办？当然只能是用其他方法lock住bus了（例如mutex，具体实现是和irq chip中的irq_bus_lock实
现相关）。一旦lock住了slow bus，然后就可以关闭中断了（中断状态保存在flag中）。
    解决了bus lock的疑问后，还有一个看起来奇奇怪怪的宏：IRQ_GET_DESC_CHECK_GLOBAL。为何在irq_set_chip函数中不设定检查（check的参数是0），而在
irq_set_irq_type接口函数中要设定global的check，到底是什么意思呢？既然要检查，那么检查什么呢？和“global”对应的不是local而是“per CPU”，内核中的宏定
义是：IRQ_GET_DESC_CHECK_PERCPU。SMP情况下，从系统角度看，中断有两种形态（或者叫mode）：
    （1）1-N mode。只有1个processor处理中断
    （2）N-N mode。所有的processor都是独立的收到中断，如果有N个processor收到中断，那么就有N个处理器来处理该中断。
    听起来有些抽象，我们还是用GIC作为例子来具体描述。在GIC中，SPI使用1-N模型，而PPI和SGI使用N-N模型。对于SPI，由于采用了1-N模型，系统（硬件加上软件）
必须保证一个中断被一个CPU处理。对于GIC，一个SPI的中断可以trigger多个CPU的interrupt line（如果Distributor中的Interrupt Processor Targets Registers
有多个bit被设定），但是，该interrupt source和CPU的接口寄存器（例如ack register）只有一套，也就是说，这些寄存器接口是全局的，是global的，一旦一个
CPU ack（读Interrupt Acknowledge Register，获取interrupt ID）了该中断，那么其他的CPU看到的该interupt source的状态也是已经ack的状态。在这种情况下，
如果第二个CPU ack该中断的时候，将获取一个spurious interrupt ID。
    对于PPI或者SGI，使用N-N mode，其interrupt source的寄存器是per CPU的，也就是每个CPU都有自己的、针对该interrupt source的寄存器接口（这些寄存器
叫做banked register）。一个CPU 清除了该interrupt source的pending状态，其他的CPU感知不到这个变化，它们仍然认为该中断是pending的。
    对于irq_set_irq_type这个接口函数，它是for 1-N mode的interrupt source使用的。如果底层设定该interrupt是per CPU的，那么irq_set_irq_type要返回错误。

---- int irq_set_chip_data(unsigned int irq, void *data)
    每个irq chip总有自己私有的数据，我们称之chip data。这个函数是用来设定该irq number的chip data

---- void __irq_set_handler(unsigned int irq, irq_flow_handler_t handle, int is_chained, const char *name)
    这是中断处理的核心内容，__irq_set_handler就是设定high level handler的接口函数，不过一般不会直接调用，而是通过irq_set_chip_and_handler_name或者
irq_set_chip_and_handler来进行设定。
    理解这个函数的关键是在is_chained这个参数。这个参数是用在interrupt级联的情况下。例如中断控制器B级联到中断控制器A的第x个interrupt source上。那么对于
A上的x这个interrupt而言，在设定其IRQ handler参数的时候要设定is_chained参数等于1，由于这个interrupt source用于级联，因此不能probe、不能被request（已经
被中断控制器B使用了），不能被threaded（具体中断线程化的概念在其他文档中描述）
