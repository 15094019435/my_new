/*
*利用新内核的驱动注册方式写的点灯驱动
*作者：李珺珺
*版本：0.2
*创建时间：2018年3月29日22点20分
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

/*下面两行是自动设置设备节点的*/
static struct class *firstdrv_class;
static struct class_device	*firstdrv_class_dev;
/*灯操作寄存器*/
volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;
/*定义一个驱动对象*/
static struct cdev firstdrv_dev;
#define  FIRST_MAJOR  230   /*定义主设备号*/
static int major = FIRST_MAJOR;

static int first_drv_open(struct inode *inode, struct file *file)
{	
	/* 配置GPF4,5,6为输出 */
	*gpfcon &= ~((0x3<<(4*2)) | (0x3<<(5*2)) | (0x3<<(6*2)));
	*gpfcon |= ((0x1<<(4*2)) | (0x1<<(5*2)) | (0x1<<(6*2)));

	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	//printk("first_drv_write\n");

	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		// 点灯
		*gpfdat &= ~((1<<4) | (1<<5) | (1<<6));
	}
	else
	{
		// 灭灯
		*gpfdat |= (1<<4) | (1<<5) | (1<<6);
	}
	
	return 0;
}

static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   first_drv_open,     
	.write	=	first_drv_write,	   
};


static int first_drv_init(void)
{
	dev_t devid;

	if(0 == major){	
		alloc_chrdev_region(&devid, 0, 1, "first_drv");
		major = MAJOR(devid);  /*得到自动分配的主设备号*/
	}
	else{
		devid = MKDEV(major, 0);
		register_chrdev_region(major, 1, "first_drv");
	}
	cdev_init(&firstdrv_dev, &first_drv_fops);
	cdev_add(&firstdrv_dev, devid, 1);

	/*自动创建设备节点*/
	firstdrv_class = class_create(THIS_MODULE, "firstdrv");
	firstdrv_class_dev = class_device_create(firstdrv_class, NULL, devid, NULL, "xyz"); /* /dev/xyz */

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	return 0;
}

static void first_drv_exit(void)
{
	cdev_del(&firstdrv_dev);/*删除设备这个对象*/
	unregister_chrdev_region(MKDEV(major, 0), 1);/*注销设备*/
	/*删除设备节点*/
	class_device_unregister(firstdrv_class_dev);
	class_destroy(firstdrv_class);
	iounmap(gpfcon);/*释放虚拟内存*/
}

module_init(first_drv_init);
module_exit(first_drv_exit);


MODULE_LICENSE("GPL");

