bootz
    bootz 用于启动 zImage 镜像文件
    bootz [addr [initrd[:size]] [fdt]]
    命令 bootz 有三个参数， addr 是 Linux 镜像文件在 DRAM 中的位置， initrd 是 initrd 文件在DRAM 中的地址，如果不使用 initrd 的话使用‘-’代
替即可， fdt 就是设备树文件在 DRAM 中的地址。如：bootz 80800000 – 83000000


bootm
    bootm 用于启动 uImage 镜像文件
    bootm [addr [initrd[:size]] [fdt]]
    其中 addr 是 uImage 在 DRAM 中的首地址， initrd 是 initrd 的地址， fdt 是设备树(.dtb)文件在 DRAM 中的首地址，如果 initrd 为空的话，同样
是用“-”来替代。
    

boot
    boot 会读取环境变量 bootcmd 来启动 Linux 系统


zImage 和 uImage 的区别
    内核编译（make）之后会生成两个文件，一个Image，一个zImage，其中Image为内核映像文件，而zImage为内核的一种映像压缩文件，Image大约为4M，而zImage
不到2M。那么uImage又是什么的？它是uboot专用的映像文件，它是在zImage之前加上一个长度为64字节的“头”，说明这个内核的版本、加载位置、生成时间、大小等
信息；其0x40之后与zImage没区别。
    
64字节的头结构如下：
typedef struct image_header {
    uint32 _tih_magic;
    uint32 _tih_hcrc;
    uint32 _tih_time;
    uint32 _tih_size;
    uint32 _tih_load;
    uint32 _tih_ep;
    uint32 _tih_dcrc;
    uint8 _tih_os;
    uint8 _tih_arch;
    uint8 _tih_type;
    uint8 _tih_comp;
    uint8 _tih_name[IH_NMLEN];
} image_header_t;