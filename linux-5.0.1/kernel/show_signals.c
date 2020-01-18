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
        printk("%d\t %d\n", rs->sig_num, rs->handled);
    }
}

asmlinkage long sys_print_signals(int pid)
{
    struct task_struct *p;
    printk("SYSCALL print signals for porcess pid:%d", pid);
    // print_signals(current);
    p = find_task_by_vpid((pid_t) pid);
    print_signals(current);

    printk("\n");
	return 0;
}
