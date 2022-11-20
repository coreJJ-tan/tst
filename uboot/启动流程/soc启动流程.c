    Romboot ->  SPL -> uboot -> linux kernel -> File system -> User APP

    一般 MCU 内部还有个固化的引导程序(Romboot)，这段程序的会初始化部分外设以与外部通信
    uboot分为uboot-spl和uboot两个组成部分。
    SPL是Secondary Program Loader的简称，第二阶段程序加载器，这里所谓的第二阶段是相对于SOC中的ROMCODE来说的，SOC启动最先执行的是ROMCODE中的固化程序。ROMCODE会通过检测启动方
式来加载第二阶段bootloader（SPL）。
    uboot已经是一个bootloader了，那么为什么还多一个uboot SPL呢？主要原因是对于一些SOC来说，它的内部SRAM可能会比较小，小到无法装载下一个完整的uboot镜像，那么就需要SPL，它主要
负责初始化外部RAM和环境，并加载真正的uboot镜像到外部RAM中来执行。所以由此来看，SPL是一个非常小的loader程序，可以运行于SOC的内部SRAM中，它的主要功能就是加载真正的uboot并运行之。
    启动阶段，芯片内部的ROMCODE首先将FLASH(eMMC/NAND)中的SPL加载到IRAM，然后直接跳转到SPL运行，然后由SPL将uboot从FLASH上搬移到DDR上，并运行之。
    CPU刚上电，需要小心的设置好很多状态，包括cpu状态、中断状态、MMU状态等等。在armv7架构的uboot-spl，主要完成以下功能：
（1）完成arch相关的初始化；
    关闭中断，svc模式
    禁用MMU、TLB
（2）SOC/BOARD相关初始化
    芯片级、板级的一些初始化操作
    IO初始化
    时钟
    选项，串口初始化
（3）初始化外部DDR；
（4）初始化FLASH（eMMC/NAND）接口；
（5）将UBOOT从FLASH（eMMC/NAND）加载到DDR；
（6）跳转到uboot处执行；

    在 U-Boot 源码中，启动过程没有完全单独出 SPL 的代码，而是复用了大量 U-Boot 里面的代码。在代码中，通过宏 CONFIG_SPL_XXX 来进行区分。因此，SPL 的启动 与 U-Boot 的启动流程
是一样的。

    一、SPL编译
    在编译SPL的时候，编译参数会有如下语句：
    u-boot/scripts/Makefile.spl
    KBUILD_CPPFLAGS += -DCONFIG_SPL_BUILD

    在编译SPL的代码的过程中，CONFIG_SPL_BUILD这个宏是打开的。uboot-spl和uboot的代码是通用的，其区别就是通过CONFIG_SPL_BUILD宏来进行区分的。
    uboot-spl编译链接脚本位于：u-boot/arch/arm/cpu/u-boot-spl.lds；从脚本中可以看出，uboot-spl 的代码入口函数是 _start 对应于路径 u-boot/arch/arm/lib/vector.S 的 _start
