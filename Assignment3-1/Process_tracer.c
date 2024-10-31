#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h> // pid_task
#include <linux/highmem.h>
#include <linux/kallsyms.h> /* kallsyms_lookup_name() */

void **syscall_table; // system call table

void *real_os_ftrace; // need to save for restoring

// tracing process(task)
asmlinkage pid_t process_tracer(const struct pt_regs *regs)
{
	// find task_struct by pid(regs->di)
	struct task_struct	* trace_ptr = pid_task(find_vpid((pid_t)regs->di), PIDTYPE_PID);
	struct task_struct	* tmp_ptr;
	int cnt; // for child, sibling count

	if (!trace_ptr) // there is no task_struct for the pid -> error
		return -1;
	// use pid, comm in task_struct for print pid, process name
	printk(KERN_INFO "##### TASK INFORMATION of ''[%d] %s'' #####\n",
		trace_ptr->pid, trace_ptr->comm);

	// print state of the process - use state, exit_state in task_struct
	if (!trace_ptr->state)
		printk(KERN_INFO "- task state : Running or ready\n");
	else if (trace_ptr->state & TASK_INTERRUPTIBLE)
		printk(KERN_INFO "- task state : Wait\n");
	else if (trace_ptr->state & TASK_UNINTERRUPTIBLE)
		printk(KERN_INFO "- task state : Wait with ignoring all signals\n");
	else if (trace_ptr->state & TASK_STOPPED)
		printk(KERN_INFO "- task state : Stopped\n");
	else if (trace_ptr->exit_state & EXIT_DEAD)
		printk(KERN_INFO "- task state : Dead\n");
	else if (trace_ptr->exit_state & EXIT_ZOMBIE)
		printk(KERN_INFO "- task state : Zombie process\n");
	else
		printk(KERN_INFO "- task state : etc.\n");
	
	// print group_leader's pid, process name by struct * group_leader
	printk(KERN_INFO "- Process Group Leader : [%d] %s\n", trace_ptr->group_leader->pid, trace_ptr->group_leader->comm);
	
	// print context switch count(voluntary + involuntary) by nvcsw, nivcsw
	printk(KERN_INFO "- Number of context switches : %d\n", trace_ptr->nvcsw + trace_ptr->nivcsw);
	
	// print fork count by using fork_count(made)
	printk(KERN_INFO "- Number of calling fork() : %d\n", trace_ptr->fork_count);

	// print parent process by struct * real_parent
	printk(KERN_INFO "- it's parent process : [%d] %s\n", trace_ptr->real_parent->pid, trace_ptr->real_parent->comm);
	
	// print sibling processes by struct list_head sibling
	printk(KERN_INFO "- it's sibling process(es) :\n");
	cnt = 0;

	// start from parent->children(== sibling)
	list_for_each_entry(tmp_ptr, &trace_ptr->real_parent->children, sibling) {
		// except for trace_ptr
		if (tmp_ptr != trace_ptr) {
			printk(KERN_INFO "   > [%d] %s\n", tmp_ptr->pid, tmp_ptr->comm);
			cnt++;
		}
	}
	
	// print the number of sibling process(es)
	if (cnt == 0)
		printk(KERN_INFO "   > It has no sibling.\n");
	else
		printk(KERN_INFO "   > This process has %d sibling process(es)\n", cnt);
	
	// print child process(es) by children
	printk(KERN_INFO "- it's child process(es) :\n");
	cnt = 0;

	// start from process(pid)->children
	list_for_each_entry(tmp_ptr, &trace_ptr->children, sibling) {
		printk(KERN_INFO "   > [%d] %s\n", tmp_ptr->pid, tmp_ptr->comm);
		cnt++;
	}

	// print the number of child process(es)
	if (cnt == 0)
		printk(KERN_INFO "   > It has no child.\n");
	else
		printk(KERN_INFO "   > This process has %d child process(es)\n", cnt);
	printk(KERN_INFO "##### END OF INFORMATION #####\n");

	// return pid -> success
	return (pid_t)regs->di;
}


/* make_rw: give READ & WRITE permission */
void make_rw(void *addr)
{
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);

	if(pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW;
}

/* make_ro: retrieve READ & WRITE permission */
void make_ro(void *addr)
{
	unsigned int level;
	pte_t *pte = lookup_address((u64)addr, &level);

	pte->pte = pte->pte &~ _PAGE_RW;
}

static int __init hooking_init(void)
{
	/* Find system call table */
	syscall_table = (void**) kallsyms_lookup_name("sys_call_table");

	/*
	 * Change permission of the page of system call table
	 * to both readable and writable
	 */
	make_rw(syscall_table);

	real_os_ftrace = syscall_table[__NR_os_ftrace]; // store the real function address
	syscall_table[__NR_os_ftrace] = process_tracer; // replace(hooking)

	/* Recover the page's permission(i.e. read-only */
	make_ro(syscall_table);
	return 0;
}

static void __exit hooking_exit(void)
{
	/*
	 * Change permission of the page of system call table
	 * to both readable and writable
	 */
	make_rw(syscall_table);
	syscall_table[__NR_os_ftrace] = real_os_ftrace; // restore the real one

	/* Recover the page's permission(i.e. read-only */
	make_ro(syscall_table);
}

module_init(hooking_init);
module_exit(hooking_exit);
MODULE_LICENSE("GPL");
