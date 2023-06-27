http://www.wowotech.net/irq_subsystem/High_level_irq_event_handler.html
1、前言
    当外设触发一次中断后，一个大概的处理过程是：
    （1）具体CPU architecture相关的模块会进行现场保护，然后调用machine driver对应的中断处理handler
    （2）machine driver对应的中断处理handler中会根据硬件的信息获取HW interrupt ID，并且通过irq domain模块翻译成IRQ number
    （3）调用该IRQ number对应的high level irq event handler，在这个high level的handler中，会通过和interupt controller交互，进行中断处理的
flow control（处理中断的嵌套、抢占等），当然最终会遍历该中断描述符的IRQ action list，调用外设的specific handler来处理该中断
    （4）具体CPU architecture相关的模块会进行现场恢复。

2、如何进入high level irq event handler
    要想分析下去，就需要了解什么是 high level irq event handler，其实就是中断描述符中的 handle_irq 成员函数（struct irq_desc -> handle_irq）,
了解了之后，接下来就是看中断发生后，如何一步步执行 high level irq event handler 的。
    从硬件上来看，中断过来，最先收到信号的是中断控制器，紧接着中断控制器将中断传递给CPU，从本质上来说，处理中断的是CPU，这时候问题就来了，CPU处理
中断的第一条指令在哪里？这个问题先不深究，实际上，在内核代码里面，处理中断是从一段汇编代码开始的，以ARM为例，该汇编代码如下：
/* Interrupt handling. */
    .macro  irq_handler
#ifdef CONFIG_MULTI_IRQ_HANDLER
    ldr r1, =handle_arch_irq
    mov r0, sp
    adr lr, BSYM(9997f)
    ldr pc, [r1]
#else
    arch_irq_handler_default
#endif
9997:
    .endm
    以上代码看出，如果配置了MULTI_IRQ_HANDLER的话，ARM中断处理则直接跳转到 handle_arch_irq() 函数执行，否则执行 arch_irq_handler_default() 函
数。有了目标，那就开干吧。

2.1 handle_arch_irq 及 arch_irq_handler_default 处理函数
（1） handle_arch_irq
    查看内核代码， handle_arch_irq并不是直接定义的一个函数，而是一个函数指针，在系统上电过程中被指定的，具体是在set_handle_irq函数或者setup_arch
函数中中被指定，为什么是两个地方指定呢？
    因为set_handle_irq函数方式指定是针对系统中只有一种interrupt controller类型的，当使用多种类型的interrupt controller的时候（例如HW 系统使用了
S3C2451这样的SOC，这时候，系统有两种interrupt controller，一种是GPIO type，另外一种是SOC上的interrupt controller），则不适合在interrupt controller
中进行设定，这时候，可以考虑在machine driver中设定。在这种情况下，handle_arch_irq 这个函数是在setup_arch函数中根据machine driver设定，下面只介绍
只有一种interrupt controller类型的情况。
#ifdef CONFIG_MULTI_IRQ_HANDLER
void __init set_handle_irq(void (*handle_irq)(struct pt_regs *))
{
    if (handle_arch_irq)
        return;
    handle_arch_irq = handle_irq;
}
#endif
    set_handle_irq函数具体是由相应的中断控制器在初始化时调用的，以GIC中断控制器为例，handle_arch_irq在 gic_init_bases()初始化函数中被指定为
gic_handle_irq函数。也就是中断发生后，会进入该函数执行。


（2） arch_irq_handler_default
    这个函数是一段汇编，还不了解，接下来的分析都是基于定义了 CONFIG_MULTI_IRQ_HANDLER 的。

2.2 gic_handle_irq函数
static void __exception_irq_entry gic_handle_irq(struct pt_regs *regs)
{
    u32 irqstat, irqnr;
    struct gic_chip_data *gic = &gic_data[0];  // 获取root GIC的硬件描述符
    void __iomem *cpu_base = gic_data_cpu_base(gic); // 获取root GIC mapping到CPU地址空间的信息
    do {
        irqstat = readl_relaxed(cpu_base + GIC_CPU_INTACK); // 获取HW interrupt ID
        irqnr = irqstat & GICC_IAR_INT_ID_MASK;
        if (likely(irqnr > 15 && irqnr < 1021)) {   // SPI和PPI的处理
            handle_domain_irq(gic->domain, irqnr, regs);    // 将HW interrupt ID转成IRQ number 并处理该IRQ number=
            continue;
        }
        if (irqnr < 16) {   // IPI类型的中断处理
            writel_relaxed(irqstat, cpu_base + GIC_CPU_EOI);
#ifdef CONFIG_SMP
            handle_IPI(irqnr, regs);
#endif
            continue;
        }
        break;
    } while (1);
}

2.3 handle_domain_irq函数