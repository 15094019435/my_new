#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

static int __init hello_init(void)
{
	printk("Hello!\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk("再见\n");
}


module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("lijunjun");            
MODULE_DESCRIPTION("hello Driver"); 
MODULE_LICENSE("GPL");                        


