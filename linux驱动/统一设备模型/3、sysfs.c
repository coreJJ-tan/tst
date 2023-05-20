涉及文件：
fs/sysfs/...
include/linux/sysfs.h

1、sysfs是什么？有什么用？
    sysfs - 用于导出内核对象(kobject)的文件系统
    首先，sysfs是一个基于RAM的文件系统，它和Kobject一起，可以将Kernel的数据结构导出到用户空间，以文件目录结构的形式，提供对这些数据结构（以及数据结构的属性）的访问支持。很重要的一点是，
sysfs具备文件系统的所有属性。sysfs对应根目录下的/sys文件夹。
    只要内核配置中定义了 CONFIG_SYSFS ，sysfs 总是被编译进内核。你可通过以下命令挂载它:
    mount -t sysfs sysfs /sys
    
    任何 kobject 在系统中注册，就会有一个目录在 sysfs 中被创建。这个目录是作为该 kobject 的父对象所在目录的子目录创建的，以准确地传递内核的对象层次到用户空间。
    属性应为 ASCII 码文本文件。以一个文件只存储一个属性值为宜。但一个文件只包含一个属性值可能影响效率，所以一个包含相同数据类型的属性值数组也被广泛地接受。

2、sysfs和Kobject的关系
    每一个Kobject，都会对应sysfs中的一个目录。因此在将Kobject添加到Kernel时，create_dir接口会调用sysfs文件系统的创建目录接口，创建和Kobject对应的目录。create_dir 接口定义在
./lib/kobject.c 文件中，被 static 修饰。

3、attribute的概念
    在sysfs中，为什么会有attribute的概念呢？其实它是对应kobject而言的，指的是kobject的“属性”。我们知道，sysfs中的目录描述了kobject，而kobject是特定数据类型变量（如struct device）的体
现。因此kobject的属性，就是这些变量的属性。它可以是任何东西，名称、一个内部变量、一个字符串等等。而attribute，在sysfs文件系统中是以文件的形式提供的，即：kobject的所有属性，都在它对应的
sysfs目录下以文件的形式呈现。这些文件一般是可读、写的，而kernel中定义了这些属性的模块，会根据用户空间的读写操作，记录和返回这些attribute的值。
    所谓的attibute，就是内核空间和用户空间进行信息交互的一种方法。例如某个driver定义了一个变量，却希望用户空间程序可以修改该变量，以控制driver的运行行为，那么就可以将该变量以sysfs attribute
的形式开放出来。
    Linux内核中，attribute分为普通的 attribute 和 bin_attribute;
    struct attribute 和 struct bin_attribute 结构体都定义在 include/linux/sysfs.h 文件中，具体如下：
struct attribute { //个人认为一个struct attribute对应一个文件，而文件的读写接口则包含在其上层的数据结构中，如struct kobj_attribute中
    const char      *name;  // name表示文件名称
    umode_t         mode;   // mode表示文件模式
    #ifdef CONFIG_DEBUG_LOCK_ALLOC
        bool            ignore_lockdep:1;
        struct lock_class_key   *key;
        struct lock_class_key   skey;
    #endif
};
struct bin_attribute {
    struct attribute    attr;
    size_t          size;
    void            *private;
    ssize_t (*read)(struct file *, struct kobject *, struct bin_attribute *, char *, loff_t, size_t);
    ssize_t (*write)(struct file *, struct kobject *, struct bin_attribute *, char *, loff_t, size_t);
    int (*mmap)(struct file *, struct kobject *, struct bin_attribute *attr, struct vm_area_struct *vma);
};
    一个单独的属性结构（struct attribute或struct bin_attribute）并不包含读写其属性值的方法。子系统最好为增删特定对象类型的属性定义自己的属性结构体和封装函数。例如kobj_attribute
结构体就包含了一个 struct attribute属性的成员
    struct attribute为普通的attribute，使用该attribute生成的sysfs文件，只能用字符串的形式读写。而struct bin_attribute在struct attribute的基础上，增加了read、write等函数，
因此它所生成的sysfs文件可以用任何方式读写。

4、attribute文件的创建
    看到 struct attribute 结构体，可能会有疑问，该接口没有read或者write接口，那么attribute文件的操作是怎么进行的呢？
    其实，所有需要使用attribute的模块，都不会直接定义struct attribute变量，而是通过一个自定义的数据结构，该数据结构的一个成员是struct attribute类型的变量，并提供show和store回调
函数。然后在该模块ktype所对应的struct sysfs_ops变量中，实现该本模块整体的show和store函数，并在被调用时，转接到自定义数据结构（struct class_attribute）中的show和store函数中。这
样，每个atrribute文件，实际上对应到一个自定义数据结构变量中了。



100、问题：
kobject配合sysfs在根文件系统下的体现？如何使用？
Kernel怎么把attribute变成sysfs中的文件呢？
用户空间对sysfs的文件进行的读写操作，怎么传递给Kernel呢？
udevadm monitor ---- ?
