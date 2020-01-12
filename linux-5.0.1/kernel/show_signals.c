#include <asm/current.h>

#include <linux/kernel.h>
#include <linux/signal_types.h>
#include <linux/sched.h>
#include <linux/list.h>

static void print_signals(struct task_struct *t)
{
    struct received_signal *rs;
    printk("signal_num\t handled\n");
	list_for_each_entry(rs, &t->rec_sig, list)
    {
        printk("%d\t %d\n");
    }
}

asmlinkage long sys_print_signals(void)
{
    printk("SYSCALL_PRINT_SIGNALS!\n\n");
    print_signals(current);
	return 0;
}
