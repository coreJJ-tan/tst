https://blog.csdn.net/yue530tomtom/article/details/76095739
1、概述
    硬件中断发生频繁，是件很消耗 CPU 资源的事情，在多核 CPU 条件下如果有办法把大量硬件中断分配给不同的 CPU (core) 处理显然能很好的平衡性能。
kernel 2.4 以后的版本利用可编程中断控制器的特性支持把不同的硬件中断请求（IRQs）分配到特定的 CPU 上，这个绑定技术被称为 SMP IRQ Affinity

2、使用前提
    （1）需要多CPU的系统，使用uname -a可以查询是否支持SMP
    （2）需要大于等于2.4的Linux 内核 ，使用uname -r可以查询内核版本
    （3）相关设置文件，设置/proc/irq/{IRQ number}/smp_affinity和/proc/irq/{IRQ number}/smp_affinity_list指定了哪些CPU能够关联到一个给
定的IRQ源. 这两个文件包含了这些指定cpu的cpu位掩码(smp_affinity)和cpu列表(smp_affinity_list)

3、用户态绑定硬件中断到某个CPU
    （1）手动绑定 IRQ 到不同 CPU，需要先停掉 IRQ 自动调节的服务进程，否则自己手动绑定做的更改将会被自动调节进程给覆盖掉。
        / # service irqbalance status // 查看 irqbalance 状态
        / # systemctl stop irqbalance // 关闭 irqbalance 自动分配服务
    （2）修改 smp_affinity 文件，这个文件中，所表示的CPU核心以十六进制来表示的。
        / # echo 4 > /proc/irq/36/smp_affinity // 指定 CPU2 来处理 IRQ number 为 36 的中断请求
    具体参见 tst/文件系统/proc文件系统/irq/smp_affinity文件.c

4、内核态 smp irq affinity 相关操作
    irq_set_affinity 函数