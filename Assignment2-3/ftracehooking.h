#ifndef FTRACEHOOKING_H
# define FTRACEHOOKING_H

# include <linux/module.h>
# include <linux/highmem.h>
# include <linux/kallsyms.h> /* kallsyms_lookup_name() */

extern pid_t	cur_pid;    // for process id check
extern char	file_name[50];  // to get file name from openat()
extern int	read_b;         // to get read bytes from read()
extern int	write_b;        // to get write bytes from write()
extern int	o_cnt;          // for openat() count
extern int	c_cnt;          // same (for close)
extern int	r_cnt;
extern int	w_cnt;
extern int	l_cnt;

# endif
