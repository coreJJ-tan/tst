涉及文件: 
lib/kobject.c
include/linux/kobject.h

1、kobject是什么? 为什么需要kobject? kset是什么?为什么需要kset?
when kobj->ktype->show and store can be called?


2、kobject的功能
    (1) 通过parent指针, 可以将所有Kobject以层次结构的形式组合起来. 
    (2) Kobject的核心功能是：保持一个引用计数, 当该计数减为0时, 自动释放 (由本文所讲的kobject模块负责)  Kobject所占用的meomry空间. 这就决定了Kobject必须是动态分配的 (只有这样才能动态释放). 
    (3) 和sysfs虚拟文件系统配合, 将每一个Kobject及其特性, 以文件的形式, 开放到用户空间. 
    注1: 在Linux中, Kobject几乎不会单独存在. 它的主要功能, 就是内嵌在一个大型的数据结构中, 为这个数据结构提供一些底层的功能实现. 
    注2: Linux driver开发者, 很少会直接使用Kobject以及它提供的接口, 而是使用构建在Kobject之上的设备模型接口. 

3、kobject、kset、ktype三个数据结构的关系
    Kobject是基本数据类型, 每个Kobject都会在"/sys/"文件系统中以目录的形式出现. 
    Ktype代表Kobject (严格地讲, 是包含了Kobject的数据结构) 的属性操作集合 (由于通用性, 多个Kobject可能共用同一个属性操作集, 因此把Ktype独立出来了) . 
    Kset是一个特殊的Kobject (因此它也会在"/sys/"文件系统中以目录的形式出现) , 它用来集合相似的Kobject (这些Kobject可以是相同属性的, 也可以不同属性的) . 

    Kobject大多数的使用场景, 是内嵌在大型的数据结构中 (如Kset、device_driver等) , 因此这些大型的数据结构, 也必须是动态分配、动态释放的. 那么释放的时机是什么呢?是内嵌的Kobject释放时. 
但是Kobject的释放是由Kobject模块自动完成的 (在引用计数为0时) , 那么怎么一并释放包含自己的大型数据结构呢?
    这时Ktype就派上用场了. 我们知道, Ktype中的release回调函数负责释放Kobject (甚至是包含Kobject的数据结构) 的内存空间, 那么Ktype及其内部函数, 是由谁实现呢?是由上层数据结构所在的模块！
因为只有它, 才清楚Kobject嵌在哪个数据结构中, 并通过Kobject指针以及自身的数据结构类型, 找到需要释放的上层数据结构的指针, 然后释放它. 
    讲到这里, 就清晰多了. 所以, 每一个内嵌Kobject的数据结构, 例如kset、device、device_driver等等, 都要实现一个Ktype, 并定义其中的回调函数. 同理, sysfs相关的操作也一样, 必须经过ktype
的中转, 因为sysfs看到的是Kobject, 而真正的文件操作的主体, 是内嵌Kobject的上层数据结构!
    顺便提一下, Kobject是面向对象的思想在Linux kernel中的极致体现, 但C语言的优势却不在这里, 所以Linux kernel需要用比较巧妙 (也很啰嗦) 的手段去实现, 

4、kobject的分配和释放
    kobject的分配有两种方式：
    （1）通过kmalloc自行分配（一般是跟随上层数据结构分配），并在初始化后添加到kernel。这种方法涉及如下接口：
    kobject_init();
    kobject_add()
    kobject_init_and_add();
    这种方式分配的kobject，会在引用计数变为0时，由kobject_put调用其ktype的release接口，释放内存空间

    （2）使用kobject_create创建
    Kobject模块可以使用kobject_create自行分配空间，并内置了一个ktype（dynamic_kobj_ktype），用于在计数为0是释放空间。
    

9、重要的结构体
struct kobject {
    const char      *name;  //该Kobject的名称, 同时也是sysfs中的目录名称. 由于Kobject添加到Kernel时, 需要根据名字注册到sysfs
                            //中, 之后就不能再直接修改该字段. 如果需要修改Kobject的名字, 需要调用kobject_rename接口, 该接口会主动处理sysfs的相关事宜. 
    struct list_head    entry; // 用于将Kobject加入到Kset中的list_head
    struct kobject      *parent; // 指向parent kobject, 以此形成层次结构 (在sysfs就表现为目录结构) . 
    struct kset     *kset; // 该kobject属于的Kset. 可以为NULL. 如果存在, 且没有指定parent, 则会把Kset作为parent
    struct kobj_type    *ktype; // 该Kobject属于的kobj_type. 每个Kobject必须有一个ktype, 或者Kernel会提示错误. 
    struct kernfs_node  *sd; // 该Kobject在sysfs中的表示. 
    struct kref     kref;
    #ifdef CONFIG_DEBUG_KOBJECT_RELEASE
        struct delayed_work release;
    #endif
    unsigned int state_initialized:1;   //指示该Kobject是否已经初始化
    unsigned int state_in_sysfs:1;      //指示该Kobject是否已在sysfs中呈现, 以便在自动注销时从sysfs中移除. 
    unsigned int state_add_uevent_sent:1;   // state_add_uevent_sent/state_remove_uevent_sent记录是否已经向用户空间发送ADD uevent, 如果有, 且没有发送remove uevent, 
                                            // 则在自动注销时, 补发REMOVE uevent, 以便让用户空间正确处理. 
    unsigned int state_remove_uevent_sent:1;
    unsigned int uevent_suppress:1;     // 如果该字段为1, 则表示忽略所有上报的uevent事件. 
};

struct kset {
    struct list_head list; // 用于保存该kset下所有的kobject的链表. 这是该kobject群的头
    spinlock_t list_lock;
    struct kobject kobj; // 该kset自己的kobject
    const struct kset_uevent_ops *uevent_ops;   //该kset的uevent操作函数集. 当任何Kobject需要上报uevent时, 都要调用它所从属的kset的uevent_ops, 添加环境变量, 或者过滤
                                            //event (kset可以决定哪些event可以上报) . 因此, 如果一个kobject不属于任何kset时, 是不允许发送uevent的. 
};

struct kobj_type {
    void (*release)(struct kobject *kobj); // 通过该回调函数, 可以将包含该种类型kobject的数据结构的内存空间释放掉. 
    const struct sysfs_ops *sysfs_ops; // 该种类型的Kobject的sysfs文件系统接口. 
    struct attribute **default_attrs; // 默认要创建的属性文件，在创建该kobject目录时就会同步创建这些文件，这对于多个 kobject 想要创建相同的属性时很重要，只要拥有同一个 
                                      // kobj_type即可，能根据此创建，不需要冗余的代码（该成员指向的指针数组要保证最后一个指针指向NULL）
    const struct kobj_ns_type_operations *(*child_ns_type)(struct kobject *kobj); 
    const void *(*namespace)(struct kobject *kobj);
};

struct kobj_attribute {
    struct attribute attr;
    ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr, char *buf); 
    ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count); 
    // buf缓冲区应总是 PAGE_SIZE 大小。该缓冲区由内核分配。
    // show() 方法应该返回写入缓冲区的字节数，也就是 snprintf()的返回值。
    // store() 应返回缓冲区的已用字节数。如果整个缓存都已填满，只需返回 count 参数。
    // show() 或 store() 可以返回错误值。当得到一个非法值，必须返回一个错误值。
};

struct sysfs_ops { //这两个函数分别在读写sysfs文件系统属性文件时得到调用
    ssize_t (*show)(struct kobject *, struct attribute *, char *);
    ssize_t (*store)(struct kobject *, struct attribute *, const char *, size_t);
};



10、kobject常用接口
void kobject_init(struct kobject *kobj, struct kobj_type *ktype)    // 初始化一个kobject, 调用后需要调用kobject_add()函数将该kobject添加进一个kobject层次结构中
{
     (1) 若kobject已经初始化, 则会报错
     (2) 将形参ktype赋给kobj->ktype
     (3) INIT_LIST_HEAD(&kobj->entry);
        kobj->state_in_sysfs = 0;   //刚初始化, kobject还没呈现在sysfs中, 所以赋值为0
        kobj->state_add_uevent_sent = 0;
        kobj->state_remove_uevent_sent = 0;
        kobj->state_initialized = 1;    //标志着该kobject已经初始化
}
int kobject_add(struct kobject *kobj, struct kobject *parent, const char *fmt, ...) // 将kobject添加到一个parent下的层次结构中
{
    @fmt: 格式化的形式为该kobject起名, 如果kobj->name原本已经本设置，则设置名字不会生效
     (1) 若kobject没初始化, 则报错返回 -EINVAL
     (2) 若parent不为 NULL, 则 kobj->parent = parent, 即添加到parent下的层次结构中, 在根文件系统体现为/sys/.../parent下的一个目录
        若parent为 NULL, 则 kobj->parent = kobj->kset->kobj, 在根文件系统体现为/sys/.../kobj->kset->kobj下的一个目录
        若kobj->kset 和 parent 都为 NULL, 则kobj->parent = NULL, 该kobject将放于sysfs的根层次结构下, 在根文件系统体现为/sys下的一个目录
     (3) 若kobj->kset != NULL, 那么将该kobj加到kobj->kset的链表中
     (4) 调用create_dir()函数在/sys下创建目录, 设置kobj->state_in_sysfs = 1;
}
struct kobject *kobject_get(struct kobject *kobj) // 该函数会返回kobj本身
{
    既然会返回kobj本身, 那么为什么创造这个函数多此一举呢? 是因为要将kobject的引用计数 (kobj->kref) 加1, 避免直接使用时引用计数为0遭到内核释放该结构体, 不要直接使用kobj. 
}
void kobject_put(struct kobject *kobj) // 将kobject的引用计数 (kobj->kref) 减1
{
    在引用计数为0时，调用ktype的release接口，释放占用空间。（release接口由驱动工程师实现，一般就是用来释放kobject结构的，其实根据项目需要，可以做相应的修改）
}
int kobject_init_and_add(struct kobject *kobj, struct kobj_type *ktype, struct kobject *parent, const char *fmt, ...) // 相当于调用kobject_init()之后调用kobject_add()
int kobject_rename(struct kobject *kobj, const char *new_name) // 为kobj改名
{
    会同步修改sysfs中目录的名字, 不要直接修改kobj->name
}
int kobject_move(struct kobject *kobj, struct kobject *new_parent) // 更改kobj->parent, 会同步修改sysfs下的文件夹的位置
void kobject_del(struct kobject *kobj) //删除kobj, 会同步删除sysfs下的文件夹
char *kobject_get_path(struct kobject *kobj, gfp_t gfp_mask) // 获取kobject对应的文件路径
{
    若该路径不再使用, 要调用kfree()释放
}
struct kobject *kobject_create(void) // 创建一个kobject初始化后返回
{
     (1) 调用kzalloc()函数申请的kobject内存, 如果该kobject不再使用, 要调用kobject_put()释放, 而不是kfree()，kobject_put本质是减小引用计数，在引用计数为0时，会自动释放。
     (2) 函数内部调用了kobject_init()进行初始化, kobj_type设置为dynamic_kobj_ktype
}
struct kobject *kobject_create_and_add(const char *name, struct kobject *parent) // 相当于调用kobject_create()之后调用kobject_add()
void kset_init(struct kset *k)
int kset_register(struct kset *k)
void kset_unregister(struct kset *k)
struct kobject *kset_find_obj(struct kset *kset, const char *name) // 在kset中搜索名为name的kobject, 返回NULL表示没找到
struct kset *kset_create_and_add(const char *name, const struct kset_uevent_ops *uevent_ops, struct kobject *parent_kobj)
const void *kobject_namespace(struct kobject *kobj)

其它比较陌生的接口: 
int kobj_ns_type_register(const struct kobj_ns_type_operations *ops)
int kobj_ns_type_registered(enum kobj_ns_type type)
const struct kobj_ns_type_operations *kobj_child_ns_ops(struct kobject *parent)
const struct kobj_ns_type_operations *kobj_ns_ops(struct kobject *kobj)
bool kobj_ns_current_may_mount(enum kobj_ns_type type)
void *kobj_ns_grab_current(enum kobj_ns_type type)
const void *kobj_ns_netlink(enum kobj_ns_type type, struct sock *sk)
const void *kobj_ns_initial(enum kobj_ns_type type)
void kobj_ns_drop(enum kobj_ns_type type, void *ns)

11、结构体成员的回调函数
static struct kobj_type dynamic_kobj_ktype = {
	.release	= dynamic_kobj_release,
	.sysfs_ops	= &kobj_sysfs_ops,
};
const struct sysfs_ops kobj_sysfs_ops = { //该结构体变量被EXPORT到内核中, 有多处调用
	.show	= kobj_attr_show,   // 后面再研究
	.store	= kobj_attr_store,  // 后面再研究
};

12、kobject使用方法
    Kobject大多数情况下（有一种例外，下面会讲）会嵌在其它数据结构中使用，其使用流程如下：
    定义一个struct kset类型的指针，并在初始化时为它分配空间，添加到内核中
    根据实际情况，定义自己所需的数据结构原型，该数据结构中包含有Kobject
    定义一个适合自己的ktype，并实现其中回调函数
    在需要使用到包含Kobject的数据结构时，动态分配该数据结构，并分配Kobject空间，添加到内核中
    每一次引用数据结构时，调用kobject_get接口增加引用计数；引用结束时，调用kobject_put接口，减少引用计数
    当引用计数减少为0时，Kobject模块调用ktype所提供的release接口，释放上层数据结构以及Kobject的内存空间
 
    上面有提过，有一种例外，Kobject不再嵌在其它数据结构中，可以单独使用，这个例外就是：开发者只需要在sysfs中创建一个目录，而不需要其它的kset、ktype的操作。这时可以直接调用
kobject_create_and_add接口，分配一个kobject结构并把它添加到kernel中。


附录（函数细节）：


100、不理解的地方
