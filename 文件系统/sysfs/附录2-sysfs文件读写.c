    打开文件时操作和普通文件一样，用户不会感受到和普通文件有什么差异，只是sysfs不会申请任何内存空间来保存文件的内容。事实上在不对文件进行操作时，文件是不存在的。只有用户读或写文件
时，sysfs才会申请一页内存，用于保存将要读取的文件信息。如果是读操作，会由VFS转到sysfs_file_operations的read（也就是sysfs_read_file）接口（4.15内核换转到sysfs_kf_read函数）上，
层层调用，直到调用文件的父对象（文件夹kobject）的属性处理函数kobject->ktype->sysfs_ops->show()，然后通过show函数来调用包含该对象的文件设备的属性的show函数（注意：这里是两个不
同的show函数）来获取硬件设备对应的属性值，然后将该值拷贝到用户空间的buff，这样就完成了读操作，写操作对应的是store函数，内核驱动编写者需要做的是实现属性文件的show()和store()函数 。

从linux3.14开始，sysfs已经采用新的kernfs框架实现。蜗窝这篇文章参考的是3.13之前的内核代码
Documentation/zh_CN/filesystems/sysfs.txt 此文件细读