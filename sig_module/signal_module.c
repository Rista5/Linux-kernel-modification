#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
// #include <linux/fs.h>
#include <linux/signal.h>
#include <linux/signal_types.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/pid.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Uros Vukic");
MODULE_DESCRIPTION("Simple module to print received signals of a process.");
MODULE_VERSION("1.0.0");

#define DEVICE_NAME "signal_module"
#define PROCFS_NAME "sig_mod"
#define MSG_BUFFER_LEN 1024
#define MSG_PRINTED 1
#define MSG_NOT_PRINTED 0

static struct proc_dir_entry *module_proc_file;
static char msg_buffer[MSG_BUFFER_LEN];
static const char HEADER_MSG[] = "SIG\tHANDLED\n";


static struct task_struct *p;
static struct list_head *ptr;
static struct list_head *head;
static struct received_signal *rs;
static int initial_msg_printed = MSG_NOT_PRINTED;

static int pid = 0;
module_param(pid, int, 0660);
MODULE_PARM_DESC(pid, "PID of a process whose signals will be printed.");

static ssize_t procfile_read(struct file *flip, char __user *ubuf, size_t count, loff_t *ppos)
{
    int bytes_read = 0;

    printk(KERN_INFO "procfile_read (/proc/%s) called\n", PROCFS_NAME);

    if (pid == 0)
    {
        printk("Process PID not set!\n");
    }
    if (p == NULL)
    {
        printk("Process with pid: %d not found\n", pid);
    }
    if (ptr == head)
    {
        printk("Head equal to ptr\n");
        ptr = head->next;
        initial_msg_printed = MSG_NOT_PRINTED;
        return 0;
    }
    if (initial_msg_printed == MSG_NOT_PRINTED)
    {
        initial_msg_printed = MSG_PRINTED;
        printk("Printing signals for process PID: %d\n", pid);
        bytes_read = strlen(HEADER_MSG);
        if(raw_copy_to_user(ubuf, HEADER_MSG, bytes_read))
            return -EFAULT;
        return bytes_read;
    }

    rs = container_of(ptr, struct received_signal, list);
    bytes_read = sprintf(msg_buffer, "%d\t%d\n", rs->sig_num, rs->handled);
    if(raw_copy_to_user(ubuf, msg_buffer, bytes_read))
        return -EFAULT;
    printk("read: %d\n", bytes_read);

    ptr = ptr->next;
    return bytes_read;
}

static ssize_t proc_write(struct file *flip, const char __user *ubuf, size_t count, loff_t *ppos)
{
    char buffer[100];
    int scaned_num, c;
    printk("Write called\n");
    if(raw_copy_from_user(buffer, ubuf, count))
        return -EINVAL;
    printk("Read from user\n");
    scaned_num = sscanf(buffer, "%d", &pid);
    printk("Scaned pid: %d\n", pid);
    printk("scaned number: %d\n", scaned_num);
    if(scaned_num != 1)
    {
        printk("Error while parsing pid\n");
        return -EINVAL;
    }
    c = strlen(buffer);
    printk("buffer len: %d\n", c);

    p = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (p != NULL)
    {
        head = &p->rec_sig;
        ptr = head->next;
        initial_msg_printed = MSG_NOT_PRINTED;
        printk("Process with PID: %d found\n", pid);
    }
    else
    {
        printk("Process with PID: %d not found!\n.", pid);
        return 0;
    }

    *ppos = c;
    return c;
}

static const struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .read = procfile_read,
    .write = proc_write,
};


static void print_sig(struct task_struct *t)
{
    struct received_signal *rs;
    printk("signal_num\t handled\n");
	list_for_each_entry(rs, &t->rec_sig, list)
    {
        printk("%d\t %d\n", rs->sig_num, rs->handled);
    }
}

static int __init init_signal_module(void)
{
    module_proc_file = proc_create(PROCFS_NAME, 0664, NULL, &file_ops);
    if (module_proc_file == NULL)
    {
        proc_remove(module_proc_file);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }
    
    if (pid == 0)
    {
        printk("Process PID not passed.\nInitialization finished\n");
        return 0;
    }
    p = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (p == NULL)
    {
        printk("Process with PID: %d not found\nnInitialization finished\n", pid);
        return 0;
    }
    print_sig(p);
    return 0;
}

static void __exit exit_signal_module(void)
{
    proc_remove(module_proc_file);
    printk("SIGNAL MODULE exit\n");
}

module_init(init_signal_module);
module_exit(exit_signal_module);