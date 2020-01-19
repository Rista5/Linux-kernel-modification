#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/signal.h>
#include <linux/signal_types.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/pid.h>
#include <asm/uaccess.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Uros Vukic");
MODULE_DESCRIPTION("Simple module to print received signals of a process.");
MODULE_VERSION("1.0.0");

#define DEVICE_NAME "signal_module"
#define EXAMPLE_MSG "Hello from signal module\n";
#define MSG_BUFFER_LEN 1024
#define MSG_PRINTED 1
#define MSG_NOT_PRINTED 0

// Prototypes for device functions
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static int major_num;
static int device_open_count = 0;
static char msg_buffer[MSG_BUFFER_LEN];


static struct task_struct *p;
static struct list_head *ptr;
static struct list_head *head;
static struct received_signal *rs;
static int initial_msg_printed = MSG_NOT_PRINTED;

static int pid = 0;
module_param(pid, int, 0774);
MODULE_PARM_DESC(pid, "PID of a process whose signals will be printed.");


// stucture with pointers to all device functions
static struct file_operations file_ops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset)
{
    int bytes_read = 0;
    size_t msg_len = 0;
    if (pid == 0)
    {
        printk("Process PID not set!\n");
    }
    if (p == NULL)
    {
        printk("Process with PID: %d not found or file not opened\n.", pid);
    }
    if (ptr == head)
    {
        return 0;
    }
    // else if (initial_msg_printed == MSG_NOT_PRINTED)
    // {
        
    // }
    rs = container_of(ptr, struct received_signal, list);
    sprintf(msg_buffer, "%d\t%d", rs->sig_num, rs->handled);
    msg_len = strlen(msg_buffer);
    while(len && bytes_read < msg_len)
    {
        put_user(msg_buffer[bytes_read], buffer++);
        bytes_read++;
        len--;
    }

    ptr = ptr->next;
    return bytes_read;
}

static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset)
{
    printk(KERN_ALERT "This operation is not supported.\n");
    return -EINVAL;
}

static int device_open(struct inode *node, struct file *flip)
{
    if (device_open_count)
    {
        return -EBUSY;
    }
    if(pid == 0)
    {
        printk("Pid not set\n");
        return -EBUSY;
    }

    device_open_count++;
    try_module_get(THIS_MODULE);
    p = pid_task(find_vpid(pid), PIDTYPE_PID);
    initial_msg_printed = MSG_NOT_PRINTED;
    if (p != NULL)
    {
        head = &p->rec_sig;
        ptr = head->next;
    }

    return 0;
}

static int device_release(struct inode *node, struct file *flip)
{
    device_open_count--;
    module_put(THIS_MODULE);
    return 0;
}

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
    major_num = register_chrdev(0, "signal_module", &file_ops);
    if(major_num < 0)
    {
        printk(KERN_ALERT "Could not register device: %d\n", major_num);
        return major_num;
    }
    else 
    {
        printk(KERN_INFO"signal_module module loaded with device major number %d\n", major_num);
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
    printk("SIGNAL MODULE exit\n");
}

module_init(init_signal_module);
module_exit(exit_signal_module);