#include "ftracehooking.h"

void **syscall_table; // system call table

void *real_os_ftrace; // need to save for restoring

pid_t	cur_pid;	 // explained in ftracehooking.h
char	file_name[50];
int		read_b;
int		write_b;
int		o_cnt;
int		c_cnt;
int		r_cnt;
int		w_cnt;
int		l_cnt;

// start or finish file tracing per one process
static asmlinkage int my_ftrace(const struct pt_regs *regs)
{
	if ((pid_t)regs->di == 0) { // parameter pid == 0
		printk(KERN_INFO "[2020202034] %s file[%s] stats [x] read - %d / written - %d\n",
				current->comm, file_name, read_b, write_b); // file information
		printk(KERN_INFO "open[%d] close[%d] read[%d] write[%d] lseek[%d]\n",
				o_cnt, c_cnt, r_cnt, w_cnt, l_cnt); // the number of syscall called
		printk(KERN_INFO "OS Assignment2 ftrace [%d] End\n", cur_pid); // end of tracing
		return 0;
	}
	read_b = 0; // initialization for new process tracing
	write_b = 0;
	o_cnt = 0;
	c_cnt = 0;
	r_cnt = 0;
	w_cnt = 0;
	l_cnt = 0;
	cur_pid = (pid_t)regs->di; // store tracing pid
	printk(KERN_INFO "OS Assignment2 ftrace [%d] Start\n", cur_pid); // format from assignment
	return	cur_pid;
}

EXPORT_SYMBOL(cur_pid); // export variables to outside
EXPORT_SYMBOL(file_name);
EXPORT_SYMBOL(read_b);
EXPORT_SYMBOL(write_b);
EXPORT_SYMBOL(o_cnt);
EXPORT_SYMBOL(c_cnt);
EXPORT_SYMBOL(r_cnt);
EXPORT_SYMBOL(w_cnt);
EXPORT_SYMBOL(l_cnt);

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
	syscall_table[__NR_os_ftrace] = my_ftrace; // replace(hooking)

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
