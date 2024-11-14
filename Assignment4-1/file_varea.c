#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h> // pid_task
#include <linux/highmem.h>
#include <linux/kallsyms.h> /* kallsyms_lookup_name() */

void **syscall_table; // system call table

void *real_os_ftrace; // need to save for restoring


asmlinkage pid_t file_varea(const struct pt_regs *regs)
{
	// find task_struct by pid(regs->di)
	struct task_struct	* pcb = pid_task(find_vpid((pid_t)regs->di), PIDTYPE_PID);
	// use for file name: d_path //
	char buf[100];
	struct file *f;
	char *file_name;
	//////
	struct vm_area_struct *vm; // pointer for virtual memory spaces

	if (!pcb) // there is no task_struct for the pid -> error
		return -1;
	// use pid, comm in task_struct for print pid, process name
	printk(KERN_INFO "######## Loaded files of a process \'%s(%d)\' in VM ########\n",
		pcb->comm, pcb->pid);

	// mm->mmap: first vm_area struct pointer
	vm = pcb->mm->mmap;
	while (vm != NULL) { // at thd end, vm will be NULL
		f = vm->vm_file; // sometimes, vm_file often be NULL
		if (f == NULL)
			file_name = "no file";
		else
			file_name = d_path(&(f->f_path), buf, sizeof(buf));
		
		// print memory informations
		printk(KERN_INFO " mem(%lx~%lx) code(%lx~%lx) data(%lx~%lx) heap(%lx~%lx) %s\n",
			vm->vm_start, vm->vm_end, vm->vm_mm->start_code, vm->vm_mm->end_code,
			vm->vm_mm->start_data, vm->vm_mm->end_data, vm->vm_mm->start_brk, vm->vm_mm->brk,
			file_name);
		vm = vm->vm_next; // move to next vm
	}
	printk(KERN_INFO "#############################################################\n");
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
	syscall_table[__NR_os_ftrace] = file_varea; // replace(hooking)

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
