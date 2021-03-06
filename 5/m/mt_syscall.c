#define __KERNEL_SYSCALLS__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/mm.h>

#include <linux/stat.h>
MODULE_DESCRIPTION("mysyscall");
MODULE_AUTHOR("WOLF");
MODULE_LICENSE("GPL");

int errno;

int __init init_testsyscall(void)
{
	_syscall0(int,getpid)
	/*_syscall1(int,do_pfcount,struct  timeval*,tv)*/
	struct timeval tv;
	/*printk(KERN_ALERT "xxx:%lu\n",pfcount);*/
	/*printk(KERN_ALERT "xxx:%lu\n",do_pfcount(&tv));*/
	/*printk(KERN_ALERT "xxx:%lu\n",tv.tv_sec);*/
	printk(KERN_ALERT "xxx:%lu\n",getpid());
	return 0;
}

void __exit exit_testsyscall(void)
{
	printk(KERN_ALERT "xxx:Goodbye\n");
}

module_init(init_testsyscall);
module_exit(exit_testsyscall);
