#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SID");
MODULE_DESCRIPTION("First attempt");

static int __init hello_init(void)
{
	printk(KERN_INFO "<1>HelloWorld\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "<1>Bye\n");
}

module_init(hello_init);
module_exit(hello_exit);
