#include <linux/kernel.h>

asmlinkage long sys_hello(void)
{
	printk("Hello world\n");
	printk("Just testing hello world!\n");
	return 0;
}
