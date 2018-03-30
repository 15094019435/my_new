/*
*�������ں˵�����ע�᷽ʽд�ĵ������
*���ߣ���B�B
*�汾��0.2
*����ʱ�䣺2018��3��29��22��20��
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

/*�����������Զ������豸�ڵ��*/
static struct class *firstdrv_class;
static struct class_device	*firstdrv_class_dev;
/*�Ʋ����Ĵ���*/
volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;
/*����һ����������*/
static struct cdev firstdrv_dev;
#define  FIRST_MAJOR  230   /*�������豸��*/
static int major = FIRST_MAJOR;

static int first_drv_open(struct inode *inode, struct file *file)
{	
	/* ����GPF4,5,6Ϊ��� */
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
		// ���
		*gpfdat &= ~((1<<4) | (1<<5) | (1<<6));
	}
	else
	{
		// ���
		*gpfdat |= (1<<4) | (1<<5) | (1<<6);
	}
	
	return 0;
}

static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   first_drv_open,     
	.write	=	first_drv_write,	   
};


static int first_drv_init(void)
{
	dev_t devid;

	if(0 == major){	
		alloc_chrdev_region(&devid, 0, 1, "first_drv");
		major = MAJOR(devid);  /*�õ��Զ���������豸��*/
	}
	else{
		devid = MKDEV(major, 0);
		register_chrdev_region(major, 1, "first_drv");
	}
	cdev_init(&firstdrv_dev, &first_drv_fops);
	cdev_add(&firstdrv_dev, devid, 1);

	/*�Զ������豸�ڵ�*/
	firstdrv_class = class_create(THIS_MODULE, "firstdrv");
	firstdrv_class_dev = class_device_create(firstdrv_class, NULL, devid, NULL, "xyz"); /* /dev/xyz */

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	return 0;
}

static void first_drv_exit(void)
{
	cdev_del(&firstdrv_dev);/*ɾ���豸�������*/
	unregister_chrdev_region(MKDEV(major, 0), 1);/*ע���豸*/
	/*ɾ���豸�ڵ�*/
	class_device_unregister(firstdrv_class_dev);
	class_destroy(firstdrv_class);
	iounmap(gpfcon);/*�ͷ������ڴ�*/
}

module_init(first_drv_init);
module_exit(first_drv_exit);


MODULE_LICENSE("GPL");

