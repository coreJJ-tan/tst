1、Uevent的功能
    Uevent是Kobject的一部分，用于在Kobject状态发生改变时，例如增加、移除等，通知用户空间程序。用户空间程序收到这样的事件后，会做相应的处理。
    设备模型中任何设备有事件需要上报时，会触发Uevent提供的接口。Uevent模块准备好上报事件的格式后，可以通过两个途径把事件上报到用户空间：一种是通
过kmod模块，直接调用用户空间的可执行文件；另一种是通过netlink通信机制，将事件从内核空间传递给用户空间。

2、uevent相关的常量和数据结构
enum kobject_action { // uevent事件类型
    KOBJ_ADD,   // Kobject（或上层数据结构）的添加/移除事件。
    KOBJ_REMOVE, // 同上
    KOBJ_CHANGE,    // Kobject（或上层数据结构）的状态或者内容发生改变。
    KOBJ_MOVE,  // Kobject（或上层数据结构）更改名称或者更改Parent（意味着在sysfs中更改了目录结构）。
    KOBJ_ONLINE,    // Kobject（或上层数据结构）的上线/下线事件，其实是是否使能。
    KOBJ_OFFLINE,   //同上
    KOBJ_MAX
};

    前面有提到过，在利用Kmod向用户空间上报event事件时，会直接执行用户空间的可执行文件。而在Linux系统，可执行文件的执行，依赖于环境变量，因此
kobj_uevent_env 用于组织此次事件上报时的环境变量。
struct kobj_uevent_env {
    char *argv[3];
    char *envp[UEVENT_NUM_ENVP]; // 指针数组，用于保存每个环境变量的地址；  #define UEVENT_NUM_ENVP         32 /* number of env pointers */
    int envp_idx;   // 用于访问环境变量指针数组的index
    char buf[UEVENT_BUFFER_SIZE]; // 保存环境变量的buffer；     #define UEVENT_BUFFER_SIZE      2048 /* buffer for the variables */
    int buflen; // buf 的实际长度
};
struct kset_uevent_ops {    // kset_uevent_ops是为kset量身订做的一个数据结构，里面包含filter和uevent两个回调函数
    int (* const filter)(struct kset *kset, struct kobject *kobj); // 当任何Kobject需要上报uevent时，它所属的kset可以通过该接口过滤，阻
    // 止不希望上报的event，从而达到从整体上管理的目的。 如果filter()函数执行返回0，则表明跳过本次uevent事件上报，参见kobject_uevent_env()函数
    // 该函数可以不实现，那么上报uevent事件的时候就不会过滤任何事件了。
    const char *(* const name)(struct kset *kset, struct kobject *kobj); // 该接口可以返回kset的名称。如果一个kset没有合法的名称，则其下
    // 的所有Kobject将不允许上报uvent，参见kobject_uevent_env()函数
    int (* const uevent)(struct kset *kset, struct kobject *kobj, struct kobj_uevent_env *env); // 当任何Kobject需要上报uevent时，它所
    // 属的kset可以通过该接口统一为这些event添加环境变量。因为很多时候上报uevent时的环境变量都是相同的，因此可以由kset统一处理，就不需要让每个
    // Kobject独自添加了。
};


单板启动过程中会调用函数 __init kobject_uevent_init() 进行初始化，进行什么工作呢？


接口：
int kobject_uevent_env(struct kobject *kobj, enum kobject_action action, char *envp_ext[])
int kobject_uevent(struct kobject *kobj, enum kobject_action action)
int add_uevent_var(struct kobj_uevent_env *env, const char *format, ...)