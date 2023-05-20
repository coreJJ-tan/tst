1、sysfs挂载在根文件系统下的/sys目录，该目录结构为：
suguoxu@sgx:~$ ls /sys
block  bus  class  dev  devices  firmware  fs  hypervisor  kernel  module  power

    /sys/devices 这是内核对系统中所有设备的分层次表达模型，是sysfs文件系统管理设备的最重要的目录结构。
    /sys/dev 这个目录维护一个按字符设备和块设备的主备号码（major:minor）链接到真实的设备（/sys/devices）符号链接文件。
    /sys/bus 这是内核设备按总线类型分层放置的目录结构，devices中的所有设备都是链接于某种总线之下，在这里的每一种具体总线之下可以找到每一个具体设备的符号链接。
    /sys/class 这是按照设备功能分类的设备模型，如系统所有输入设备都会出现在/sys/class/input之下。
    /sys/kernel 这里是内核所有可调整参数的位置（调整内核参数也在/proc/sys/kernel中）。
    /sys/module 这里有系统中所有的模块的信息，不论这些模块是以内联方式编译到内核映像文件中还是编译为外部模块。
    /sys/power 这是系统中电源选项，这个目录下有几个属性文件可以用于控制整个机器的电源状态，如可以向其中写入控制命令让机器关机、重启等。
    /sys/block 这是所有块设备文件的符号链接，连接到/sys/devices下的块设备文件。

