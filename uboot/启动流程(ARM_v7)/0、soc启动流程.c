这里先给个启动流程的结论：Romboot ->  SPL -> uboot -> linux kernel -> File system -> User APP

    针对不同架构的CPU，uboot启动流程在细节方面会有所不同，但是整体的框架上还是一致的，在此仅针对ARMv7架构的CPU启动流程细节进行分析
    正常宿主机对u-boot完成编译后，要对编译好的u-boot进行移植烧录，将一编译好的u-boot.bin文件烧写到sd卡或者是开发板的flash存储器中，启动阶段, 芯片内部的ROMCODE首先将
FLASH(eMMC/NAND)中的SPL加载到IRAM, 然后直接跳转到SPL运行, 然后由SPL将uboot从FLASH上搬移到DDR上, 并运行之。

1、第 1 步：Romboot阶段
    一般 MCU 内部还有个固化的引导程序(Romboot), 单板上电后，这段程序会初始化部分外设使得能够与外部通信，随后进行下一阶段启动。

2、 SPL 阶段
    uboot分为uboot-spl和uboot两个组成部分。SPL是Secondary Program Loader的简称, 第二阶段程序加载器, 这里所谓的第二阶段是相对于SOC中的ROMCODE来说的, SOC启动最先执行的是
ROMCODE中的固化程序。ROMCODE会通过检测启动方式来加载第二阶段bootloader(SPL)。
    uboot已经是一个bootloader了, 那么为什么还多一个uboot SPL呢？主要原因是对于一些SOC来说, 它的内部SRAM可能会比较小, 小到无法装载下一个完整的uboot镜像, 那么就需要SPL, 它主要
负责初始化外部RAM和环境, 并加载真正的uboot镜像到外部RAM中来执行。所以由此来看, SPL是一个非常小的loader程序, 可以运行于SOC的内部SRAM中, 它的主要功能就是加载真正的uboot并运行之。
    在 U-Boot 源码中, 启动过程没有完全单独出 SPL 的代码, 而是复用了大量 U-Boot 里面的代码。在代码中, 通过宏 CONFIG_SPL_XXX 来进行区分。因此, SPL 的启动 与 U-Boot 的启动流程
是一样的。至于SPL的入口在哪，可以查看spl的链接脚本（./arch/arm/cpu/u-boot-spl.lds），在链接脚本中，可以看到入口地址为 _start，这部分和uboot一致，对应于路径 
u-boot/arch/arm/lib/vector.S 中的 _start。
    CPU刚上电, 需要小心的设置好很多状态, 包括cpu状态、中断状态、MMU状态等等。在armv7架构的uboot-spl, 主要完成以下功能：
    (1)完成arch相关的初始化; 
        关闭中断, svc模式
        禁用MMU、TLB
    (2)SOC/BOARD相关初始化
        芯片级、板级的一些初始化操作
        IO初始化
        时钟
        选项, 串口初始化
    (3)初始化外部DDR; 
    (4)初始化FLASH(eMMC/NAND)接口; 
    (5)将UBOOT从FLASH(eMMC/NAND)加载到DDR; 
    (6)跳转到uboot处执行; 

SPL编译：
    在编译SPL的时候, 编译参数会有如下语句：
    u-boot/scripts/Makefile.spl
    KBUILD_CPPFLAGS += -DCONFIG_SPL_BUILD

3、uboot 阶段
    uboot的入口地址可以查看 u-boot.lds 链接脚本，与 SPL 一样，从 ./arch/arm/lib/vector.S 中的 _start地址开始运行， vector.S 文件主要是存放一堆中断向量表，_start一进来，
就通过 b reset 指令跳转到 reset 地址去执行，该处的代码位于 ./arch/arm/cpu/armv7/start.S 中，在此，仅简要说明reset开始的几个步骤，更详细的介绍待后续牛逼了再补充。reset之后
主要有如下流程：
    reset -> save_boot_params -> save_boot_params_ret -> cpu_init_cp15 -> cpu_init_crit -> lowlevel_init -> _main -> board_init_f_alloc_reserve
-> board_init_f_init_reserve -> board_init_f -> relocate_code -> relocate_vectors -> c_runtime_cpu_setup -> spl_relocate_stack_gd -> memset -> coloured_LED_init
-> red_led_on -> board_init_r -> main_loop
    上面的流程中，比较重要的有以下几个, 会分为相应的文件来介绍：
    board_init_f()
    board_init_r()
    main_loop()

4、linux kernel 阶段

5、File system 阶段

6、User APP 阶段
    
    
    

    

    

