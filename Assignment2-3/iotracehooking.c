#include "ftracehooking.h"

void **syscall_table;

// the real file funtions pointer
asmlinkage long (*real_openat)(const struct pt_regs *);
asmlinkage long (*real_close)(const struct pt_regs *);
asmlinkage long (*real_read)(const struct pt_regs *);
asmlinkage long (*real_write)(const struct pt_regs *);
asmlinkage long (*real_lseek)(const struct pt_regs *);

// hooking openat() system call
static asmlinkage long ftrace_openat(const struct pt_regs *regs) {
	// count open and store file name only if cur_pid == current pid
	if (cur_pid == current->pid) {
		o_cnt++;
		copy_from_user(file_name, (void *)regs->si,
			sizeof(file_name));
	}
	// return original open system call
	return real_openat(regs);
}

static asmlinkage long ftrace_close(const struct pt_regs *regs) {
	// count close() only if cur_pid == current pid
	if (cur_pid == current->pid)
		c_cnt++;
	return real_close(regs);
}

static asmlinkage long ftrace_read(const struct pt_regs *regs) {
	// count read and store read bytes only if cur_pid == current pid
	long	read_return = real_read(regs);
	if (cur_pid == current->pid) {
		r_cnt++;
		read_b += read_return;
	}
	return read_return;
}

static asmlinkage long ftrace_write(const struct pt_regs *regs) {
	// count write and store written bytes only if cur_pid == current pid
	long write_return = real_write(regs);
	if (cur_pid == current->pid) {
		w_cnt++;
		write_b += write_return;
	}
	return write_return;
}

static asmlinkage long ftrace_lseek(const struct pt_regs *regs) {
	// count lseek only if cur_pid == current pid
	if (cur_pid == current->pid)
		l_cnt++;
	return real_lseek(regs);
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
	syscall_table = (void **)kallsyms_lookup_name("sys_call_table");

	/*
	 * Change permission of the page of system call table
	 * to both readable and writable
	 */
	make_rw(syscall_table);

	// store the real function address and hooking
	real_openat = syscall_table[__NR_openat];
	syscall_table[__NR_openat] = (long (*)(const struct pt_regs *))ftrace_openat;

	real_close = syscall_table[__NR_close];
	syscall_table[__NR_close] = (long (*)(const struct pt_regs *))ftrace_close;

	real_read = syscall_table[__NR_read];
	syscall_table[__NR_read] = (long (*)(const struct pt_regs *))ftrace_read;

	real_write = syscall_table[__NR_write];
	syscall_table[__NR_write] = (long (*)(const struct pt_regs *))ftrace_write;

	real_lseek = syscall_table[__NR_lseek];
	syscall_table[__NR_lseek] = (long (*)(const struct pt_regs *))ftrace_lseek;

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

	// restore the real one
	syscall_table[__NR_openat] = real_openat;
	syscall_table[__NR_close] = real_close;
	syscall_table[__NR_read] = real_read;
	syscall_table[__NR_write] = real_write;
	syscall_table[__NR_lseek] = real_lseek;

	/* Recover the page's permission(i.e. read-only */
	make_ro(syscall_table);
}

module_init(hooking_init);
module_exit(hooking_exit);
MODULE_LICENSE("GPL");
