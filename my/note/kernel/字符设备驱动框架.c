#include <linux/types.h> 
#include <linux/kernel.h> 
#include <linux/delay.h> 
#include <linux/ide.h> 
#include <linux/init.h>
#include <linux/module.h> 
#include <linux/errno.h> 
#include <linux/gpio.h> 
#include <linux/cdev.h> 
#include <linux/device.h> 
#include <asm/mach/map.h> 
#include <asm/uaccess.h> 
#include <asm/io.h>

#define BSP_ERROR -1
#define BSP_OK 0

#define DEVCNT 1
 
/*设备结构体 */ 
struct chr_dev{ 
    dev_t devid; /* 设备号 */ 
    struct cdev cdev; /* cdev */ 
    struct class *class; /* 类 */
    struct device *device; /* 设备 */ 
    int major; /* 主设备号 */ 
    int minor; /* 次设备号 */ 
    int devcnt;
}; 
struct chr_dev chrtest; /* 字符设备 */ 
 
/* 
* @description : 打开设备 
* @param – inode : 传递给驱动的inode 
* @param - filp : 设备文件，file结构体有个叫做private_data的成员变量 
* 一般在open的时候将private_data指向设备结构体。 
* @return : 0 成功;其他 失败 
*/ 
static int test_open(struct inode *inode, struct file *filp) 
{ 
    filp->private_data = &chrtest; /* 设置私有数据 */ 
    return 0; 
} 
 
static ssize_t test_read(struct file *filp, char __user *buf, 
size_t cnt, loff_t *offt) 
{ 
    return 0; 
} 
 
static ssize_t test_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt) 
{ 
    return 0;
} 
 
static int test_release(struct inode *inode, struct file *filp) 
{ 
    return 0; 
} 
 
/* 设备操作函数 */ 
static struct file_operations chr_fops = { 
    .owner = THIS_MODULE, 
    .open = test_open, 
    .read = test_read, 
    .write = test_write, 
    .release = test_release, 
}; 
 
/* 
* @description : 驱动入口函数 
*/ 
static int __init test_init(void) 
{ 
    u32 val = 0; 
    int ret;
    
    chrtest.devcnt = DEVCNT;
    /* 注册字符设备驱动 */ 
    /* 1、创建设备号 */ 
    if (chrtest.major) { /* 定义了设备号 */ 
        chrtest.devid = MKDEV(chrtest.major, 0); 
        ret = register_chrdev_region(chrtest.devid, chrtest.devcnt,"test"); 
        if(ret != 0)
        {
            printk("register devid error.\n");
            goto devid_err;
        }
    } else { /* 没有定义设备号 */ 
        ret = alloc_chrdev_region(&chrtest.devid, 0, chrtest.devcnt,"test"); /* 申请设备号 */ 
        chrtest.major = MAJOR(chrtest.devid); /* 获取主设备号 */ 
        chrtest.minor = MINOR(chrtest.devid); /* 获取次设备号 */ 
        if(ret != 0)
        {
            printk("alloc devid error.\n");
            goto devid_err;
        }
    } 
    
    /* 2、初始化cdev */ 
    chrtest.cdev.owner = THIS_MODULE; 
    cdev_init(&chrtest.cdev, &chr_fops); 
    
    /* 3、添加一个cdev */ 
    cdev_add(&chrtest.cdev, chrtest.devid, 1); 
    
    /* 4、创建类 */ 
    chrtest.class = class_create(THIS_MODULE,"test"); 
    if (IS_ERR(chrtest.class)) {
        printk("class create error.\n");
		return PTR_ERR(chrtest.class);
	}
    
    /* 5、创建设备 */ 
    chrtest.device = device_create(chrtest.class, NULL, chrtest.devid, NULL,"test"); 
    if (IS_ERR(chrtest.device)) {
        printk("device create error.\n");
		return PTR_ERR(chrtest.device);
	}
    
    return 0; 
device_err:
    class_destroy(chrtest.class);

class_err:
    cdev_del(&chrtest.cdev);
    unregister_chrdev_region(chrtest.devid, chrtest.devcnt);
devid_err:
    return -1;
} 
 
/* 
* @description : 驱动出口函数 
*/ 
static void __exit test_exit(void) 
{ 
 
    /* 注销字符设备 */ 
    cdev_del(&chrtest.cdev);/* 删除cdev */ 
    unregister_chrdev_region(chrtest.devid, chrtest.devcnt); 
    device_destroy(chrtest.class, chrtest.devid); 
    class_destroy(chrtest.class); 
} 
 
module_init(test_init); 
module_exit(test_exit); 
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("wb");