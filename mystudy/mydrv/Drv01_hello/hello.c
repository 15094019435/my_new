/*         最基本的驱动练习
  没有含有自动注册设备，需要安装驱动后手动注册
*/



#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>


#define DEVICE_NAME     "hello"  /* 加载模式后，执行”cat /proc/devices”命令看到的设备名称 */
#define HELLO_MAJOR       231     /* 手动设置主设备号， 如果设为0,表示自动设置主设备号 */




/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations hello_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
};

/*
 * 执行“insmod hello.ko时就会调用这个函数
 */
static int __init hello_init(void)
{
    int ret;

    /* 注册字符设备驱动程序
     * 参数为主设备号、设备名字、file_operations结构；
     * 这样，主设备号就和具体的file_operations结构联系起来了，
     * 操作主设备为HELLO_MAJOR的设备文件时，就会调用hello_fops中的相关成员函数
     * HELLO_MAJOR可以设为0，表示由内核自动分配主设备号
     */
    ret = register_chrdev(HELLO_MAJOR, DEVICE_NAME, &hello_fops);//安装驱动
    if (ret < 0) {
      printk(DEVICE_NAME "安装驱动失败！\n");
      return ret;
    }
    
    printk(DEVICE_NAME "Hello！\n");
    return 0;
}

/*
 * 执行”rmmod hello.ko”命令时就会调用这个函数 
 */
static void __exit hello_exit(void)
{
    /* 卸载驱动程序 */
    unregister_chrdev(HELLO_MAJOR, DEVICE_NAME);
}


/* 这两行指定驱动程序的初始化函数和卸载函数 */
module_init(hello_init);
module_exit(hello_exit);

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("lijunjun");             // 驱动程序的作者
MODULE_DESCRIPTION("hello Driver");   // 一些描述信息
/*遵循的协议,必须*/
MODULE_LICENSE("GPL");                              // 遵循的协议


