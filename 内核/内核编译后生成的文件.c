    vmlinux  编译出来的最原始的内核文件，未压缩。
    zImage   是vmlinux经过gzip压缩后的文件。
    bzImage  bz表示“big zImage”，不是用bzip2压缩的。两者的不同之处在于，zImage解压缩内核到低端内存(第一个640K)，bzImage解压缩内核到高端内存(1M以上)。
如果内核比较小，那么采用zImage或bzImage都行，如果比较大应该用bzImage。
    uImage   U-boot专用的映像文件，它是在zImage之前加上一个长度为0x40的tag。
    vmlinuz  是bzImage/zImage文件的拷贝或指向bzImage/zImage的链接。
    initrd   是“initial ramdisk”的简写。一般被用来临时的引导硬件到实际内核vmlinuz能够接管并继续引导的状态。

zImage和uImage的区别:
     vmlinux是内核文件，zImage是一般情况下默认的压缩内核映像文件，压缩vmlinux，加上一段解压启动代码得到。而uImage则是使用工具mkimage对普通的压缩内
核映像文件（zImage）加工而得。它是uboot专用的映像文件，它是在zImage之前加上一个长度为64字节的“头”，说明这个内核的版本、加载位置、生成时间、大小等信
息；其0x40之后与zImage没区别。其实就是一个自动跟手动的区别,有了uImage头部的描述,u-boot就知道对应Image的信息,如果没有头部则需要自己手动去搞那些参数。
如何生成uImage文件？首先在uboot的/tools目录下寻找mkimage文件，把其copy到系统/usr/local/bin目录下，这样就完成制作工具。然后在内核目录下运行make uImage，
如果成功，便可以在arch/arm/boot/目录下发现uImage文件，其大小比 zImage多64个字节。此外，平时调试用uImage，不用去管调整了哪些东西；zImage则是一切OK后直接
烧0X0。开机就运行。