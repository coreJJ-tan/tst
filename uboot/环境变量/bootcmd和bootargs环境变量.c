1、 bootcmd
    uboot开机自动启动实际就是在内部执行了bootcmd这个环境变量的值所对应的命令集。
    bootcmd的默认值就是 CONFIG_BOOTCOMMAND 。可以在板级.h文件中通过设置宏CONFIG_BOOTCOMMAND来设置bootcmd的默认值

2、 bootargs
    bootargs保存着uboot传递给Linux内核的参数。
    bootargs的默认值就是 CONFIG_BOOTARGS 。
    下面贴出一个bootargs示例：
    bootargs=console=ttymxc0,115200 rw root=/dev/nfs nfsroot=192.168.10.102:/home/suguoxu/linux/nfs/rootfs,proto=tcp rw ip=192.168.10.111:192.168.10.102:192.168.10.1:255.255.0.0::eth0:off
    可见，bootargs 就是设置了很多的参数的值，这些参数 Linux 内核会使用到，常用的参数有：
2.1 console
    console 用来设置 linux 终端(或者叫控制台)，也就是通过什么设备来和 Linux 进行交互，是串口还是 LCD 屏幕？如果是串口的话应该是串口几等等。这里设置 console 为 ttymxc0，
ttymxc0 后面有个“,115200”，这是设置串口的波特率， console=ttymxc0,115200 综合起来就是设置 ttymxc0（也就是串口 1）作为 Linux 的终端，并且串口波特率设置为 115200。
     rw 表示根文件系统是可以读写的，不加 rw 的话可能无法在根文件系统中进行写操作，只能进行读操作。

2.2、 root
    root 用来设置根文件系统的位置：
    （1）root=/dev/nfs 用于指明根文件系统存放在nfs设备中，也就是通过网络挂载根文件系统，这在调试时十分有用。
    （2）root=/dev/mmcblk1p2 用于指明根文件系统存放在mmcblk1 设备的分区 2 中。
    有些bootargs会在 root 后面加“rootwait”， rootwait 表示等待相应的设备初始化完成以后再挂载，否则的话设备还没初始化完成就挂载根文件系统会出错的。

2.3 rootfstype
    此选项一般配置 root 一起使用， rootfstype 用于指定根文件系统类型，如果根文件系统为 ext 格式的话此选项无所谓。如果根文件系统是 yaffs、 jffs 或 ubifs 的话就需要设置此
选项，指定根文件系统的类型。

2.4 nfsroot
    如果使用nfs挂载根文件系统，那么需要设置该参数，表明nfs的的路径，后面的proto表示挂载时使用的网络协议，一般是tcp，rw表示挂载的nfs目录可读可写

2.5 ip
    设置本板的ip以及远程server的ip

2.6 其他
    还可以添加自己定义的参数，比如某些单板通过boot参数向内核传递一些信息，比如boardid等等，这是因为可能很多单板共用一个内核镜像文件，通过该参数来匹配单板