// 所谓的 platform 驱动并不是独立于字符设备驱动、块设备驱动和网络设备驱动之外的其他种类的驱动。 platform 只是为了驱动的分离与分层而提出来
// 的一种框架，其驱动的具体实现还是需要字符设备驱动、块设备驱动或网络设备驱动。


/* 设备结构体 */
struct xxx_dev{
    struct cdev cdev;
    /* 设备结构体其他具体内容 */
};

struct xxx_dev xxxdev; /* 定义个设备结构体变量 */

static int xxx_open(struct inode *inode, struct file *filp)
{
    /* 函数具体内容 */
    return 0;
}

static ssize_t xxx_write(struct file *filp, const char __user *buf,
size_t cnt, loff_t *offt)
{
    /* 函数具体内容 */
    return 0;
}

/*
* 字符设备驱动操作集
*/
static struct file_operations xxx_fops = {
    .owner = THIS_MODULE,
    .open = xxx_open,
    .write = xxx_write,
};

/*
* platform 驱动的 probe 函数
* 驱动与设备匹配成功以后此函数就会执行
*/
static int xxx_probe(struct platform_device *dev)
{
    
    cdev_init(&xxxdev.cdev, &xxx_fops); /* 注册字符设备驱动 */
    /* 函数具体内容 */
    return 0;
}

static int xxx_remove(struct platform_device *dev)
{

    cdev_del(&xxxdev.cdev);/* 删除 cdev */
    /* 函数具体内容 */
    return 0;
}

/* 匹配列表 */
static const struct of_device_id xxx_of_match[] = {
    { .compatible = "xxx-gpio" },
    { /* Sentinel */ }
};

/*
* platform 平台驱动结构体
*/
static struct platform_driver xxx_driver = {
    .driver = {
        .name = "xxx",
        .of_match_table = xxx_of_match,
    },
    .probe = xxx_probe,
    .remove = xxx_remove,
};

/* 驱动模块加载 */
static int __init xxxdriver_init(void)
{
    return platform_driver_register(&xxx_driver);
}

/* 驱动模块卸载 */
static void __exit xxxdriver_exit(void)
{
    platform_driver_unregister(&xxx_driver);
}

module_init(xxxdriver_init);
module_exit(xxxdriver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("suguoxu");