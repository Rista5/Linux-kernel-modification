# Linux kernel modification - Advanced operating systems

## Assignment
Show all signals that a process with a given PID has handled using a signal handler function

## What is a signal?
* Small message that can be sent to a process or a group of processes
* The only information being sent is the signal number 

(example from http://man7.org/linux/man-pages/man7/signal.7.html)
 Signal      Standard   Action   Comment
       ────────────────────────────────────────────────────────────────────────
       SIGABRT      P1990      Core    Abort signal from abort(3)
       SIGALRM      P1990      Term    Timer signal from alarm(2)
       SIGBUS       P2001      Core    Bus error (bad memory access)
       SIGCHLD      P1990      Ign     Child stopped or terminated
       SIGCLD         -        Ign     A synonym for SIGCHLD
       SIGCONT      P1990      Cont    Continue if stopped
       SIGEMT         -        Term    Emulator trap
       SIGFPE       P1990      Core    Floating-point exception
       SIGHUP       P1990      Term    Hangup detected on controlling terminal
                                       or death of controlling process
                                       
                                       
## How do prcesses send signals?
Kernel distinguishes two different phases related to signal transmission:
1. Signal generation - The kernel updates a data structure of the destination process to represent that a
new signal has been sent
2. Signal delivery - The kernel forces the destination process to react to the signal by changing its
execution state, by starting the execution of a specified signal handler, or both.

![Image of linux kernel strtuctires associated with signals] (https://github.com/Rista5/Linux-kernel-modification/blob/master/prezentacija/signal%20structures.png)

## Modifications

### received_signal sitructure
This structure keeps information about received signals
* sig_num - signal number / signal type
* handled - has value 0 if received signal wasn't hanled, 1 if it was
* list - list_head node, keeping pointers to adjacent nodes in a list

Link: [include/linux/signal_types.h](https://github.com/Rista5/Linux-kernel-modification/blob/ff733caa5cebf4b2d5bdfc20be95de9c65985ee4/linux-5.0.1/include/linux/signal_types.h#L71)
```c
#define SIGNAL_HANDLED 1
#define SIGNAL_NOT_HANDLED 0

struct received_signal {
	int sig_num;
	int handled;
	struct list_head list;
};
```
All recevied_signal nodes of a process are organised in a doubley linked list 
and can be accessed as a rec_sig paramter in task_struct structure. 
Link: [received_signal list in task_struct](https://github.com/Rista5/Linux-kernel-modification/blob/ff733caa5cebf4b2d5bdfc20be95de9c65985ee4/linux-5.0.1/include/linux/sched.h#L1205)

### Changes in signal.c file
This file contains complete logic for signals in linux kernel.
Two funcitons were added in this file for monitoring of received signals:
 * [add_received_signal](https://github.com/Rista5/Linux-kernel-modification/blob/ff733caa5cebf4b2d5bdfc20be95de9c65985ee4/linux-5.0.1/kernel/signal.c#L1075) - function which adds a new node in a received signal list inside task_struct structure.
It is called within [send_signal](https://github.com/Rista5/Linux-kernel-modification/blob/ff733caa5cebf4b2d5bdfc20be95de9c65985ee4/linux-5.0.1/kernel/signal.c#L1136) function. Send signal 
function implements a process of sending a signal to a process by adding new sigqueue node structure in received signal list.
```c
static void add_received_signal(int sig, struct task_struct *t)
{
	struct received_signal *rs;
	rs = kmalloc(sizeof(*rs), GFP_KERNEL);
	if(t == NULL || rs == NULL)
		return;
	rs->sig_num = sig;
	rs->handled = SIGNAL_NOT_HANDLED;
	list_add(&rs->list, &t->rec_sig);
}
```

* [change_received_signal_status](https://github.com/Rista5/Linux-kernel-modification/blob/ff733caa5cebf4b2d5bdfc20be95de9c65985ee4/linux-5.0.1/kernel/signal.c#L2597) - function which changes a status of received signals to handled. It is called inside signal_delived function, which is called after a signal has been successfully processed.
```c
static void change_received_signal_status(int sig)
{
	struct received_signal *rs;
	list_for_each_entry(rs, &current->rec_sig, list)
	{
		if (rs->sig_num == sig && rs->handled != SIGNAL_HANDLED)
		{
			rs->handled = SIGNAL_HANDLED;
			return;
		}
	}
}

```

### Additional modifications in fork.c file
This file contains code for fokring/copying of a process and it's task_struct structure. Inside a copy_process function 
(function that performs copying of task_struct structure), [INIT_LIST_HEAD](https://github.com/Rista5/Linux-kernel-modification/blob/ff733caa5cebf4b2d5bdfc20be95de9c65985ee4/linux-5.0.1/kernel/fork.c#L2130) is called to initialize rec_sig list of received signals.
[free_task_struct](https://github.com/Rista5/Linux-kernel-modification/blob/ff733caa5cebf4b2d5bdfc20be95de9c65985ee4/linux-5.0.1/kernel/fork.c#L172) - function that clears task_struct when process is killed. It was modified to also remove  received_signal nodes from received signal list.

### [System call](https://github.com/Rista5/Linux-kernel-modification/blob/ff733caa5cebf4b2d5bdfc20be95de9c65985ee4/linux-5.0.1/kernel/show_signals.c#L1) for printing received signal list
System call has one parameter - PID of a process for which to print the list. It simply traverses signal list and prints out its information in kernel log.
```c
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
```
## [Module implementation](https://github.com/Rista5/Linux-kernel-modification/blob/ff733caa5cebf4b2d5bdfc20be95de9c65985ee4/sig_module/signal_module.c#L1)
Module is implemented as a device to whom we can access through file sistem. Access is the same as the access to proc file system.
To achive this funcitonality, module implements part of interface given with file_opeartions structure>
* owner - owner of a file
* read - pointer to a function for reading a file
* write - pointer to a function for writing to a file
```c
static const struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .read = procfile_read,
    .write = proc_write,
};
```

Module has only one parametar - PID which determins a process whose list to read. This parameter can be passed through 
initialization of a module or through a write function.
If parametar was passed through module initialization, after initialization, module will print signals for process whose PID
was passed in kernel log.

```c
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
```
