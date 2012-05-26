#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

/*包含了所有的系统调用号__NR__...*/
#include <linux/unistd.h>

/*for struct time*/
#include <linux/time.h>

/*for copy_to_user()*/
#include <asm/uaccess.h>

/*for current marcro*/
#include <linux/sched.h>

#include <linux/syscalls.h>

#define __NR_mysyscall 349

MODULE_DESCRIPTION("mysyscall");
MODULE_AUTHOR("Wolf");
MODULE_LICENSE("GPL");

/* 用来保存旧系统调用的地址 */
static int(*anything_saved)(void);

unsigned long **find_sys_call_table(void) {
    unsigned long **sctable;
    unsigned long ptr;
    extern int loops_per_jiffy;

    sctable = NULL;
    for (ptr = (unsigned long)&loops_per_jiffy;
        ptr < (unsigned long)&boot_cpu_data; ptr += sizeof(void *)){
   
        unsigned long *p;
        p = (unsigned long *)ptr;
        if (p[__NR_close] == (unsigned long) sys_close){
            sctable = (unsigned long **)p;
            return &sctable[0];
        }
    }

    return NULL;
}

unsigned long **sys_call_table ;


/* 这个是我们自己的系统调用函数 sys_mysyscall*/
static int sys_mysyscall(struct timeval *tv)
{
	struct timeval ktv;

	/*增加模块使用计数*/
	/*MOD_INC_USE_COUNT;*/

	do_gettimeofday(&ktv);
	if (copy_to_user(tv,&ktv,sizeof(ktv))){
		/*MOD_DEC_USE_COUNT;*/
		return -EFAULT;
	}

	printk(KERN_ALERT "Pid %ld called sys_gettimeofday().\n",(long)current->pid);
	/*MOD_DEC_USE_COUNT;*/

	return 0;
}

/*初始化函数*/
int __init init_addsyscall(void)
{	
	//2.6内核不再导出sys_call_table
	//extern long sys_call_table[];
	sys_call_table = find_sys_call_table();	

	/*保存原来系统调用表中此位置的系统调用*/
	anything_saved = (int(*)(void))(sys_call_table[__NR_mysyscall]);

	/* 把我们自己的系统调用放入系统调用表，注意进行类型转换*/
	sys_call_table[__NR_mysyscall] = (unsigned long)sys_mysyscall;

	return 0;
}

void __exit exit_addsyscall(void)
{
	//extern long sys_call_table[];

	/*恢复原先的系统调用 */
	sys_call_table[__NR_mysyscall] = (unsigned long)anything_saved;
}

/*两个宏告诉系统真正的初始化和推出函数*/
module_init(init_addsyscall);
module_exit(exit_addsyscall);
