#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/pid.h> // pid_task
#include <linux/proc_fs.h>
#include <linux/string.h> // strlen
#include <linux/uaccess.h> // copy_to_user(), copy_from_user()

struct proc_dir_entry *fp_dir; // for directory
struct proc_dir_entry *fp_file; // for file

#define BUF_SIZE 4096

long cur_proc = -1; // store writed process pid(ex> echo 1 > /proc... )
int is_processed = 0;

// open file function //
int my_open(struct inode *inode, struct file *file)
{
    is_processed = 0;
    return 0;
}

// if the file read, my_read will return cur_proc's information or all info' //
ssize_t my_read(struct file *f, char __user *data_usr, size_t len, loff_t *off)
{
    char *buffer;
    char buf[512];
    long buf_size = BUF_SIZE;
    ssize_t len_written;
    struct task_struct	*ptr;

    if(is_processed) return 0;

    // allocate buffer for copy_to_user //
    if (!(buffer = kmalloc(buf_size, GFP_KERNEL)))
        return 0;
    buffer[0] = '\0'; // for strcat()

    // if no selected process, or pid(cur_proc) not exist, ptr == NULL //
    if (cur_proc <= 0)
        ptr = NULL;
    else
        ptr = pid_task(find_vpid(cur_proc), PIDTYPE_PID);

    // format //
    sprintf(buf, "%-8s%-8s%-8s%-8s%-20s%-20s%-20s%-20s\n", "Pid", "PPid", "Uid", "Gid", "utime", "stime", "State", "Name");
    strcat(buffer, buf);
    strcat(buffer, "--------------------------------------------------------------------------------------------------\n");
    
    // print one process' information: cur_proc //
    if (ptr)
    {
        // cred(struct): permission information //
        sprintf(buf, "%-8d%-8d%-8d%-8d%-20lu%-20lu", ptr->pid, ptr->real_parent->pid, ptr->cred->uid, ptr->cred->gid,
            ptr->utime, ptr->stime);
        strcat(buffer, buf);
        // process state //
        if (!ptr->state)
            sprintf(buf, "%-20s", "R (running)");
        else if (ptr->state & TASK_INTERRUPTIBLE)
            sprintf(buf, "%-20s", "S (sleeping)");
        else if (ptr->state & TASK_UNINTERRUPTIBLE)
            sprintf(buf, "%-20s", "D (disk sleep)");
        else if (ptr->state & TASK_STOPPED)
            sprintf(buf, "%-20s", "T (stopped)");
        else if (task_is_stopped_or_traced(ptr))
            sprintf(buf, "%-20s", "t (tracing stop)");
        else if (ptr->exit_state & EXIT_DEAD)
            sprintf(buf, "%-20s", "X (dead)");
        else if (ptr->exit_state & EXIT_ZOMBIE)
            sprintf(buf, "%-20s", "Z (zombie)");
        else if (ptr->state & TASK_PARKED)
            sprintf(buf, "%-20s", "P (parked)");
        else if (ptr->state & TASK_IDLE)
            sprintf(buf, "%-20s", "I (idle)");
        strcat(buffer, buf);
        // process name //
        sprintf(buf, "%-20s\n", ptr->comm);
        strcat(buffer, buf);
    }
    else
    {
        // for_each_process: reach all exist process //
        for_each_process(ptr)
        {
            sprintf(buf, "%-8d%-8d%-8d%-8d%-20lu%-20lu", ptr->pid, ptr->real_parent->pid, ptr->cred->uid, ptr->cred->gid,
                ptr->utime, ptr->stime);
            // print all process need lots of memory: so, if size lack, reallocate //
            if (strlen(buf) < buf_size - strlen(buffer))
                strcat(buffer, buf);
            else
            {
                if (!(buffer = krealloc(buffer, buf_size*2, GFP_KERNEL)))
                    return 0;
                buf_size *= 2; // size double
                strcat(buffer, buf);
            }
            // state //
            if (!ptr->state)
                sprintf(buf, "%-20s", "R (running)");
            else if (ptr->state & TASK_INTERRUPTIBLE)
                sprintf(buf, "%-20s", "S (sleeping)");
            else if (ptr->state & TASK_UNINTERRUPTIBLE)
                sprintf(buf, "%-20s", "D (disk sleep)");
            else if (ptr->state & TASK_STOPPED)
                sprintf(buf, "%-20s", "T (stopped)");
            else if (task_is_stopped_or_traced(ptr))
                sprintf(buf, "%-20s", "t (tracing stop)");
            else if (ptr->exit_state & EXIT_DEAD)
                sprintf(buf, "%-20s", "X (dead)");
            else if (ptr->exit_state & EXIT_ZOMBIE)
                sprintf(buf, "%-20s", "Z (zombie)");
            else if (ptr->state & TASK_PARKED)
                sprintf(buf, "%-20s", "P (parked)");
            else if (ptr->state & TASK_IDLE)
                sprintf(buf, "%-20s", "I (idle)");
            // write to buffer or size realloc //
            if (strlen(buf) < buf_size - strlen(buffer))
                strcat(buffer, buf);
            else
            {
                if (!(buffer = krealloc(buffer, buf_size*2, GFP_KERNEL)))
                    return 0;
                buf_size *= 2;
                strcat(buffer, buf);
            }
            // process name //
            sprintf(buf, "%-20s\n", ptr->comm);
            // buffer //
            if (strlen(buf) < buf_size - strlen(buffer))
                strcat(buffer, buf);
            else
            {
                if (!(buffer = krealloc(buffer, buf_size*2, GFP_KERNEL)))
                    return 0;
                buf_size *= 2;
                strcat(buffer, buf);
            }
        }
    }
    len_written = strlen(buffer)+1;
    // write to user space //
    copy_to_user(data_usr, buffer, len_written);
    
    // free buffer //
    kfree(buffer);

    is_processed++;
    return len_written; // return writed bytes
}

// when write to file(ex> echo 1 > /proc/proc~), this function called
ssize_t my_write(struct file *f, const char __user *data_usr, size_t len, loff_t *off)
{
    char buf[50];
    ssize_t len_copied;

    len_copied = len;
    // read content that user wrote //
    copy_from_user(buf, data_usr, len_copied);
    // null-terminated: to avoid kstrtol error //
    buf[len_copied-1] = '\0';
    // translate to number(long) //
    kstrtol(buf, 10, &cur_proc);

    return len_copied;
}

// if read, write syscall called, these will be called //
const struct file_operations my_ops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write,
};

int __init my_init(void)
{
    // make directory, file //
    fp_dir = proc_mkdir("proc_2020202034", NULL);
    fp_file = proc_create("processInfo", 0666, fp_dir, &my_ops);

    return 0;
}

void __exit my_exit(void)
{
    // remove directory, file //
    remove_proc_entry("processInfo", fp_dir);
    remove_proc_entry("proc_2020202034", NULL);
}

module_init(my_init); // sudo insmod [module]
module_exit(my_exit); // sudo rmmod  [module]
MODULE_LICENSE("GPL");