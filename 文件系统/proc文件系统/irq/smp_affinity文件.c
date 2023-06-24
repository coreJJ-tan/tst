1、内容
    suguoxu@sgx:/proc/irq/0$ cat smp_affinity
    ff
    suguoxu@sgx:/proc/irq/0$

2、使用方式
    该文件与 smp irq affinity 有关，具体含义请移步 "tst/专题/内核-smp irq affinity介绍.c"
    注意 smp_affinity 的内容是一个十六进制的 bitmask ，它和 cpu No.序列的“与”运算结果就是将affinity设置在那个（那些）CPU了。（也即
smp_affinity 中被设置为 1 的位为 CPU No.）
    如：8 个逻辑core，那么 CPU# 的序列为 11111111 （从右到左依次为 CPU0 ~ CPU7 ）
    如果 cat /proc/irq/76/smp_affinity 的值为： 20（20 是 16 进制对应的二进制为：00100000与11111111求与），则 76 这个 IRQ 中断上
报将被设置为 #5 号 CPU。每个 IRQ 的默认的 smp affinity 在这里： cat /proc/irq/default_smp_affinity

    目录下还有个 smp_affinity_list ，他是十进制的表达方式
    两个配置是相通的，smp_affinity_list 使用的是十进制，相比较smp_affinity的十六进制，可读性更好些。

3、使用总结
    通过手动改变smp_affinity文件中的值来将IRQ绑定到指定的CPU核心上，或者启用irqbalance服务来自动绑定IRQ到CPU核心上。没有启动
irqbalance 也没有合理的做手动 irq 绑定的话会有性能问题。手动 irq 只推荐给很 heavy、很特殊的情况，比如带多网卡多硬盘的网络存储服务器，
一般机器一般应用还是用irqbalance 省心
    在一个大量小包的系统上，irqbalance优化几乎没有效果，而且还使得cpu消耗分配的不均衡，导致机器性能得不到充分的利用，这个时候需要把它给
结束掉。
    对于文件服务器、高流量 Web 服务器这样的应用来说，把不同的网卡 IRQ 均衡绑定到不同的 CPU 上将会减轻某个 CPU 的负担，提高多个 CPU 整
体处理中断的能力；
    对于数据库服务器这样的应用来说，把磁盘控制器绑到一个 CPU、把网卡绑定到另一个 CPU 将会提高数据库的响应时间、优化性能。合理的根据自己
的生产环境和应用的特点来平衡 IRQ 中断有助于提高系统的整体吞吐能力和性能。
    DB可以设置一部分CPU处理硬盘IO，一部分CPU处理网络IO，多网卡服务器可以设置对应CPU对应网卡。当然具体情况看生产情况而定。
