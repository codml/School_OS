#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h> /* kallsyms_lookup_name() */
#include <linux/syscalls.h> /* SYSCALL_DEFINEx() */
#include <asm/syscall_wrapper.h> /* __SYSCALL_DEFINEx() */

void **syscall_table;

void *real_os_ftrace;

/**
 * Function to be hooked
 * 	- Low-level macro of the "SYSCALL_DEFINEn" (n:1-6)
 *	- Parameters of this macro
 *	  --> 1 : Number of parameters of this system call
 *	  --> my_ftrace : Name of the new system call hooked
 *	  --> pid_t : parameter type of the "my_ftrace" system call
 *	  --> pid : parameter name of the system call
 **/

__SYSCALL_DEFINEx(1, my_ftrace, pid_t, pid)
{
	printk(KERN_INFO "os_ftrace() hooked! os_ftrace -> my_ftrace\n"); // format from assignment
	return 0;
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

	real_os_ftrace = syscall_table[__NR_os_ftrace];
	syscall_table[__NR_os_ftrace] = __x64_sysmy_ftrace;

	return 0;
}

static void __exit hooking_exit(void)
{
	syscall_table[__NR_os_ftrace] = real_os_ftrace;

	/* Recover the page's permission(i.e. read-only */
	make_ro(syscall_table);
}

module_init(hooking_init);
module_exit(hooking_exit);
MODULE_LICENSE("GPL");
