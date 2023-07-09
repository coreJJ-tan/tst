http://www.wowotech.net/tty_framework/tty_driver.html
1、关键数据结构
1.1 TTY device
    Linux TTY framework 的核心功能，就是管理 TTY 设备，以方便应用程序使用。于是，问题来了，Linux kernel 是怎么抽象 TTY 设备的呢？答案很尴尬，kernel 
不认为 TTY device 是一个设备，这很好理解：
    比如，我们熟悉的串口终端，串口控制器（serial controller）是一个实实在在的硬件设备，一个控制器可以支持多个串口（serial port），软件在串口上收发数
据，就相当于在驱动“串口终端”。此处的 TTY device，就是从串口控制器中抽象出来的一个数据通道；
    再比如，我们常用的网络终端，只有以太网控制器（或者WLAN控制器）是实实在在的设备，sshd等服务进程，会基于网络socket，虚拟出来一个数据通道，软件在这个
通道上收发数据，就相当于在驱动“网络终端”。
    因此，从kernel的角度看，TTY device就是指那些“虚拟的数据通道”。
    由于 TTY driver 在 linux kernel 中出现的远比设备模型早，所以在 TTY framework 中，没有特殊的数据结构用于表示 TTY 设备。当然，为了方便，kernel 从
设备模型和字符设备两个角度对它进行了抽象：
    1）设备模型的角度
    为每个“数据通道”注册了一个stuct device，以便可以在sysfs中体现出来，例如：
    /sys/class/tty/tty
    /sys/class/tty/console
    /sys/class/tty/ttyS0

    2）字符设备的角度
    为每个“数据通道”注册一个struct cdev，以便在用户空间可以访问，例如：
    /dev/tty
    /dev/console
    /dev/ttyS0

1.2 TTY driver(struct tty_driver)
    从当前设备模型的角度看，TTY framework 有点奇怪，它淡化了 device 的概念，却着重突出 driver。由 struct tty_driver 所代表的 TTY driver，几乎大包
大揽了 TTY device 有关的所有内容，如下：
struct tty_driver {
    int magic;      /* magic number for this structure */
    struct kref kref;   /* Reference management */
    struct cdev *cdevs;
    struct module   *owner;
    const char  *driver_name;   // 该TTY driver的名称，在软件内部使用；
    const char  *name;  // 该 TTY driver 所驱动的 TTY devices 的名称，会体现到 sysfs 以及/dev/等文件系统下；
    int name_base;  /* offset of printed name */
    int major;      /* major device number */ // major、minor_start，该TTY driver所驱动的TTY devices的在字符设备中的主次设备号。因为一个tty driver
                                              // 可以支持多个tty device，因此次设备号只指定了一个start number；
    int minor_start;    /* start of minor device number */
    unsigned int    num;    /* number of devices allocated */  // 该driver所驱动的tty device的个数，可以在tty driver注册的时候指定，也可以让
                                                               // TTY core自行维护，具体由TTY_DRIVER_DYNAMIC_DEV flag决定
    short   type;       /* type of tty driver */    // type、subtype，TTY driver的类型，具体可参考“include/linux/tty_driver.h”中的定义；
    short   subtype;    /* subtype of tty driver */
    struct ktermios init_termios; /* Initial termios */ // 初始的termios
    unsigned long   flags;      /* tty driver flags */
    struct proc_dir_entry *proc_entry; /* /proc fs entry */
    struct tty_driver *other; /* only used for the PTY driver */

    /*
     * Pointer to the tty data structures
     */
    struct tty_struct **ttys; // 保存每个设备的tty_struct列表
    struct tty_port **ports;
    struct ktermios **termios;
    void *driver_state; // 可存放tty driver的私有数据。

    /*
     * Driver methods
     */

    const struct tty_operations *ops; // ops，tty driver的操作函数集
    struct list_head tty_drivers;
};

    原则上来说，在编写 TTY driver 的时候，我们只需要定义一个 struct tty_driver 变量，并根据实际情况正确填充其中的字段后，注册到 TTY core 中，即可
完成驱动的设计。

1.3 TTY struct(struct tty_struct)+
    TTY struct是TTY设备在TTY core中的内部表示。
    从TTY driver的角度看，它和文件句柄的功能类似，用于指代某个TTY设备。
    从TTY core的角度看，它是一个比较复杂的数据结构，保存了TTY设备生命周期中的很多中间变量
    编写TTY driver的时候不需要特别关心struct tty_struct的内部细节
struct tty_struct {
    int magic;
    struct kref kref;
    struct device *dev;
    struct tty_driver *driver;
    const struct tty_operations *ops; // 该设备的tty操作函数集指针；
    int index; // 该设备的编号（如tty0、tty1中的0、1）

    /* Protects ldisc changes: Lock tty not pty */
    struct ld_semaphore ldisc_sem;
    struct tty_ldisc *ldisc;

    struct mutex atomic_write_lock;
    struct mutex legacy_mutex;
    struct mutex throttle_mutex;
    struct rw_semaphore termios_rwsem;
    struct mutex winsize_mutex;
    spinlock_t ctrl_lock;
    spinlock_t flow_lock;
    /* Termios values are protected by the termios rwsem */
    struct ktermios termios, termios_locked;
    struct termiox *termiox;    /* May be NULL for unsupported */
    char name[64];
    struct pid *pgrp;       /* Protected by ctrl lock */
    struct pid *session;
    unsigned long flags;
    int count;
    struct winsize winsize;     /* winsize_mutex */
    unsigned long stopped:1,    /* flow_lock */
              flow_stopped:1,
              unused:BITS_PER_LONG - 2;
    int hw_stopped;
    unsigned long ctrl_status:8,    /* ctrl_lock */
              packet:1,
              unused_ctrl:BITS_PER_LONG - 9;
    unsigned int receive_room;  /* Bytes free for queue */
    int flow_change;

    struct tty_struct *link;
    struct fasync_struct *fasync;
    int alt_speed;      /* For magic substitution of 38400 bps */
    wait_queue_head_t write_wait;
    wait_queue_head_t read_wait;
    struct work_struct hangup_work;
    void *disc_data;
    void *driver_data;
    struct list_head tty_files;

#define N_TTY_BUF_SIZE 4096

    int closing;
    unsigned char *write_buf;
    int write_cnt;
    /* If the tty has a pending do_SAK, queue it here - akpm */
    struct work_struct SAK_work;
    struct tty_port *port;
};

1.4 TTY port(struct tty_port)
    在TTY framework中TTY port是一个比较难理解的概念，因为它和TTY struct类似，也是TTY device的一种抽象。那么，既然有了TTY struct，为什么还需要TTY port呢？.
    TTY struct是TTY设备的“动态抽象”，保存了TTY设备访问过程中的一些临时信息，这些信息是有生命周期的：从打开TTY设备开始，到关闭TTY设备结束；
    TTY port是TTY设备固有属性的“静态抽象”，保存了该设备的一些固定不变的属性值，例如是否是一个控制台设备（console）、打开关闭时是否需要一些delay操作、等等；
    另外（这一点很重要），TTY core负责的是逻辑上的抽象，并不关心这些固有属性。因此从层次上看，这些属性完全可以由具体的TTY driver自行维护；
    不过，由于不同TTY设备的属性有很多共性，如果每个TTY driver都维护一个私有的数据结构，将带来代码的冗余。所以TTY framework就将这些共同的属性抽象出来，保存
在struct tty_port数据结构中，同时提供一些通用的操作接口，供具体的TTY driver使用；
    因此，总结来说：TTY struct是TTY core的一个数据结构，由TTY core提供并使用，必要的时候可以借给具体的TTY driver使用；TTY port是TTY driver的一个数据结构，
由TTY core提供，由具体的TTY driver使用，TTY core完全不关心。
struct tty_port {
    struct tty_bufhead  buf;        /* Locked internally */
    struct tty_struct   *tty;       /* Back pointer */
    struct tty_struct   *itty;      /* internal back ptr */
    const struct tty_port_operations *ops;  /* Port operations */
    spinlock_t      lock;       /* Lock protecting tty field */
    int         blocked_open;   /* Waiting to open */
    int         count;      /* Usage count */
    wait_queue_head_t   open_wait;  /* Open waiters */
    wait_queue_head_t   close_wait; /* Close waiters */
    wait_queue_head_t   delta_msr_wait; /* Modem status change */
    unsigned long       flags;      /* TTY flags ASY_*/
    unsigned char       console:1,  /* port is a console */
                low_latency:1;  /* optional: tune for latency */
    struct mutex        mutex;      /* Locking */
    struct mutex        buf_mutex;  /* Buffer alloc lock */
    unsigned char       *xmit_buf;  /* Optional buffer */
    unsigned int        close_delay;    /* Close port delay */
    unsigned int        closing_wait;   /* Delay for output */
    int         drain_delay;    /* Set to zero if no pure time
                           based drain is needed else
                           set to size of fifo */
    struct kref     kref;       /* Ref counter */
};

1.5 termios(struct ktermios)
    在Unix/Linux的世界中，终端（terminal）编程是一个非常繁琐的事情，为了改善这种状态，特意制订了符合POSIX规范的应用程序编程接口，称作 POSIX terminal interface。
POSIX terminal interface操作的对象，就是名称为 termios 的数据结构（在用户空间为struct termios，内核空间为struct ktermios）。
    不同的 CPU 架构，struct ktermios定义不一样，以 ARM 为例：
struct ktermios {
    tcflag_t c_iflag;       /* input mode flags */ // c_ispeed、c_ospeed，可以分别控制TTY设备输入和输出的速度（例如串口设备中的波特率）；
    tcflag_t c_oflag;       /* output mode flags */
    tcflag_t c_cflag;       /* control mode flags */    // 可以控制TTY设备的一些特性，例如data bits、parity type、stop bit、flow control等（例如串口设备中经常提到的8N1）； 
    tcflag_t c_lflag;       /* local mode flags */
    cc_t c_line;            /* line discipline */
    cc_t c_cc[NCCS];        /* control characters */
    speed_t c_ispeed;       /* input speed */
    speed_t c_ospeed;       /* output speed */
};

1.6 tty driver flags
/*
 * tty driver flags
 *
 * TTY_DRIVER_RESET_TERMIOS --- requests the tty layer to reset the
 *  termios setting when the last process has closed the device.
 *  Used for PTY's, in particular.
 *
 * TTY_DRIVER_REAL_RAW --- if set, indicates that the driver will
 *  guarantee never not to set any special character handling
 *  flags if ((IGNBRK || (!BRKINT && !PARMRK)) && (IGNPAR ||
 *  !INPCK)).  That is, if there is no reason for the driver to
 *  send notifications of parity and break characters up to the
 *  line driver, it won't do so.  This allows the line driver to
 *  optimize for this case if this flag is set.  (Note that there
 *  is also a promise, if the above case is true, not to signal
 *  overruns, either.)
 *
 * TTY_DRIVER_DYNAMIC_DEV --- if set, the individual tty devices need
 *  to be registered with a call to tty_register_device() when the
 *  device is found in the system and unregistered with a call to
 *  tty_unregister_device() so the devices will be show up
 *  properly in sysfs.  If not set, driver->num entries will be
 *  created by the tty core in sysfs when tty_register_driver() is
 *  called.  This is to be used by drivers that have tty devices
 *  that can appear and disappear while the main tty driver is
 *  registered with the tty core.
 *
 * TTY_DRIVER_DEVPTS_MEM -- don't use the standard arrays, instead
 *  use dynamic memory keyed through the devpts filesystem.  This
 *  is only applicable to the pty driver.
 *
 * TTY_DRIVER_HARDWARE_BREAK -- hardware handles break signals. Pass
 *  the requested timeout to the caller instead of using a simple
 *  on/off interface.
 *
 * TTY_DRIVER_DYNAMIC_ALLOC -- do not allocate structures which are
 *  needed per line for this driver as it would waste memory.
 *  The driver will take care.
 *
 * TTY_DRIVER_UNNUMBERED_NODE -- do not create numbered /dev nodes. In
 *  other words create /dev/ttyprintk and not /dev/ttyprintk0.
 *  Applicable only when a driver for a single tty device is
 *  being allocated.
 */
#define TTY_DRIVER_INSTALLED        0x0001
#define TTY_DRIVER_RESET_TERMIOS    0x0002
#define TTY_DRIVER_REAL_RAW     0x0004
#define TTY_DRIVER_DYNAMIC_DEV      0x0008
#define TTY_DRIVER_DEVPTS_MEM       0x0010
#define TTY_DRIVER_HARDWARE_BREAK   0x0020
#define TTY_DRIVER_DYNAMIC_ALLOC    0x0040
#define TTY_DRIVER_UNNUMBERED_NODE  0x0080

    TTY_DRIVER_DYNAMIC_DEV ：如果设置了该 flag，则表示 TTY driver 会在需要的时候，自行调用 tty_register_device() 接口注册 TTY 设备（相应地回体现在字符设备
以及 sysfs 中）；如果没有设置，TTY core会 在 tty_register_driver() 时根据 driver->num 信息，自行创建对应的 TTY 设备。

1.7 TTY操作函数集 (struct tty_operations)
    TTY core将和硬件有关的操作，抽象、封装出来，形成名称为struct tty_operations的数据结构，具体的TTY driver不需要关心具体的业务逻辑，只需要根据实际的硬件情
况，实现这些操作接口即可。
struct tty_operations {
    struct tty_struct * (*lookup)(struct tty_driver *driver, struct inode *inode, int idx);
    int  (*install)(struct tty_driver *driver, struct tty_struct *tty);
    void (*remove)(struct tty_driver *driver, struct tty_struct *tty);
    int  (*open)(struct tty_struct * tty, struct file * filp);
    void (*close)(struct tty_struct * tty, struct file * filp);
    void (*shutdown)(struct tty_struct *tty);
    void (*cleanup)(struct tty_struct *tty);
    int  (*write)(struct tty_struct * tty, const unsigned char *buf, int count);  // TTY core会通过“.write“接口，将输出信息送给终端设备并显示。因此具
                                                                                  // 体的TTY driver需要实现该接口，并通过硬件操作将数据送出；
    int  (*put_char)(struct tty_struct *tty, unsigned char ch);
    void (*flush_chars)(struct tty_struct *tty);
    int  (*write_room)(struct tty_struct *tty);
    int  (*chars_in_buffer)(struct tty_struct *tty);
    int  (*ioctl)(struct tty_struct *tty, unsigned int cmd, unsigned long arg);
    long (*compat_ioctl)(struct tty_struct *tty, unsigned int cmd, unsigned long arg);
    void (*set_termios)(struct tty_struct *tty, struct ktermios * old);
    void (*throttle)(struct tty_struct * tty);
    void (*unthrottle)(struct tty_struct * tty);
    void (*stop)(struct tty_struct *tty);
    void (*start)(struct tty_struct *tty);
    void (*hangup)(struct tty_struct *tty);
    int (*break_ctl)(struct tty_struct *tty, int state);
    void (*flush_buffer)(struct tty_struct *tty);
    void (*set_ldisc)(struct tty_struct *tty);
    void (*wait_until_sent)(struct tty_struct *tty, int timeout);
    void (*send_xchar)(struct tty_struct *tty, char ch);
    int (*tiocmget)(struct tty_struct *tty);
    int (*tiocmset)(struct tty_struct *tty, unsigned int set, unsigned int clear);
    int (*resize)(struct tty_struct *tty, struct winsize *ws);
    int (*set_termiox)(struct tty_struct *tty, struct termiox *tnew);
    int (*get_icount)(struct tty_struct *tty, struct serial_icounter_struct *icount);
#ifdef CONFIG_CONSOLE_POLL
    int (*poll_init)(struct tty_driver *driver, int line, char *options);
    int (*poll_get_char)(struct tty_driver *driver, int line);
    void (*poll_put_char)(struct tty_driver *driver, int line, char ch);
#endif
    const struct file_operations *proc_fops;
};

2、 用于编写TTY driver的API
2.1  TTY driver有关的API
#define tty_alloc_driver(lines, flags) __tty_alloc_driver(lines, THIS_MODULE, flags)    // tty_alloc_driver，分配一个struct tty_driver指针，并初始化那些不需要driver关心的字段
                                                        // ines，指明该driver最多能支持多少个设备，TTY core会根据该参数，分配相应个数的ttys、ports、termios数组；
struct tty_driver *__tty_alloc_driver(unsigned int lines, struct module *owner, unsigned long flags)
void put_tty_driver(struct tty_driver *d)
void tty_set_operations(struct tty_driver *driver, const struct tty_operations *op)
struct tty_driver *tty_find_polling_driver(char *name, int *line)
void tty_driver_kref_put(struct tty_driver *driver)
int tty_register_driver(struct tty_driver *driver)  // 将TTY driver注册给TTY core。
int tty_unregister_driver(struct tty_driver *driver)

2.2 TTY device有关的API
    如果T TY driver 设置了 TTY_DRIVER_DYNAMIC_DEV flag，就需要自行注册 TTY device，相应的 API 包括：
struct device *tty_register_device(struct tty_driver *driver, unsigned index, struct device *device)    // 分配并注册一个TTY device，最后将新分配的设备指针返回给调用者：
    return tty_register_device_attr(driver, index, device, NULL, NULL); // index，该TTY设备的编号，它会决定该设备在字符设备中的设备号，以及相应的设备名称，例如/dev/ttyS0中的‘0’；
                                                                        // device，可选的父设备指针。
struct device *tty_register_device_attr(struct tty_driver *driver, unsigned index, struct device *device,
                                        void *drvdata, const struct attribute_group **attr_grp) // 和tty_register_device类似，只不过可以额外指定设备的attribute。
void tty_unregister_device(struct tty_driver *driver, unsigned index)

2.3 数据传输有关的API
    当TTY core有数据需要发送给TTY设备时，会调用TTY driver提供的.write或者.put_char回调函数，TTY driver在这些回调函数中操作硬件即可。
    当TTY driver从TTY设备收到数据并需要转交给TTY core的时候，需要调用TTY buffer有关的接口，将数据保存在缓冲区中，并等待Application读取，相关的API有：
static inline int tty_insert_flip_char(struct tty_port *port, unsigned char ch, char flag)
 static inline int tty_insert_flip_string(struct tty_port *port, const unsigned char *chars, size_t size)

3、TTY driver的编写步骤
    步骤1：实现TTY设备有关的操作函数集，并保存在一个struct tty_operations变量中。
    步骤2：调用tty_alloc_driver分配一个TTY driver，并根据实际情况，设置driver中的字段（包括步骤1中的struct tty_operations变量）。
    步骤3：调用tty_register_driver将driver注册到kernel。
    步骤4：如果需要动态注册TTY设备，在合适的时机，调用tty_register_device或者tty_register_device_attr，向kernel注册TTY设备。
    步骤5：接收到数据时，调用tty_insert_flip_string或者tty_insert_flip_char将数据交给TTY core；TTY core需要发送数据时，会调用driver提供的回调函数，在那里面访问硬件送出数据即可。