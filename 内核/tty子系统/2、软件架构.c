    本文主要介绍 TTY 子系统的相关软件架构，以及各个组成部分的作用。
1、架构框图
    linux tty设备相关代码在 drivers/tty 目录中。其软件框图如下：
           ___________________________________________________________
          |                                                           |
          |                          Application                      |
          |___________________________________________________________|
                    |            |             |              |
        /dev/console|    /dev/tty|    /dev/tty0|    /dev/ttyS0|   
                    |            |             |              |                 User Space       
        ____________|____________|_____________|______________|___________________________________
                    |            |             |              |                 Kernel Space    
           _________|____________|_____________|______________|_______
          |                                                           |
          |                     Char Device Driver                    |
          |___________________________________________________________|
                                        |
           _____________________________|_____________________________               printk        
          |                          TTY Core                         |      ___________|_________
          |   ___________________     __________     __________       |_____| System Console Core |
          |  |TTY Line Discipline|   |TTY Device|   |TTY Driver|      |     |_____________________|
          |  |___________________|   |__________|   |__________|      |
          |___________|___________________________|___________________|
                      |                           |
           ___________|__________    _____________|_________________
          | TTY Line Disciplines |  |  TTY Drivers/Console Drivers  |
          |    _____    _____    |  |   __    ______    _____       |
          |   |n_tty|  | ... |   |  |  |VT|  |Serial|  | ... |      |
          |   |_____|  |_____|   |  |  |__|  |______|  |_____|      |
          |______________________|  |_______________________________|

1.1 TTY Core
    TTY core是TTY framework的核心逻辑，功能包括：
    (1) 以字符设备的形式，向用户空间提供访问TTY设备的接口
        在 devfs 中会看到多种不同的 tty 设备，例如
        设备节点                    主次设备号        备注
        /dev/tty                     (5, 0)         控制终端（Controlling Terminal）
        /dev/console                 (5, 1)         控制台终端（Console Terminal）
        /dev/vc/0 or /dev/tty0       (4, 0)         虚拟终端（Virtual Terminal）
        /dev/vc/1 or /dev/tty1       (4, 0)         虚拟终端（Virtual Terminal）
                ...                   ...             ...
        /dev/ttyS0                   (x, x)         串口终端（名称和设备号由驱动自行决定）
            ...                       ...              ..
        /dev/ttyUSB0                 (x, x)         USB转串口终端
    (2)通过设备模型中的struct device结构抽象TTY设备，并通过struct tty_driver抽象该设备的驱动，并提供相应的register接口。TTY驱动程序的编写，简化为填
充并注册相应的struct tty_driver结构。
    ---- 提示：TTY framework弱化了TTY设备（图片1中使用虚线框标注）的概念，通常情况下，可以在注册TTY驱动的时候，自动分配并注册TTY设备。
    (3)使用 struct tty_struct、struct tty_port 等数据结构，从逻辑上抽象TTY设备及其“组件”，以实现硬件无关的逻辑。
    (4)抽象出名称为线路规程（Line Disciplines）的模块，在向TTY硬件发送数据之前，以及从TTY设备接收数据之后，进行相应的处理（如特殊字符的转换等）。

1.2 System Console Core

1.3 TTY Line Disciplines

1.4 TTY Drivers以及System Console Drivers