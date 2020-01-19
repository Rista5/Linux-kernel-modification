#include <asm/current.h>

#include <linux/kernel.h>
#include <linux/signal_types.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/syscalls.h>

static void print_sig(struct task_struct *t)
{
    struct received_signal *rs;
    printk("signal_num\t handled\n");
	list_for_each_entry(rs, &t->rec_sig, list)
    {
        printk("%d\t %d\n", rs->sig_num, rs->handled);
    }
}

SYSCALL_DEFINE1(print_signals, pid_t, pid)
{
    struct task_struct *p;
    printk("SYSCALL print signals for porcess pid: %d\n", pid);
    // print_signals(current);
    p = find_task_by_vpid(pid);
    if (p != NULL)
    {
        print_sig(p);
    }
    else
    {
        printk("Unable to find process with pid: %d\n", pid);
    }
    

    printk("\n");
	return 0;
}
