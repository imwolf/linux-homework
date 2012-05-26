/*#define __KERNEL__*/
#include <stdio.h>
/*#include <errno.h>*/
/*#include <sys/time.h>*/
/*#include <linux/unistd.h>*/
/*#include <sys/syscall.h>*/
#include <unistd.h>
#include <sys/time.h>
#include "/home/wolf/linux/linux-2.6.35.14/arch/x86/include/asm/unistd_32.h"
//系统调用入口汇编软中断宏
//2.6.X系统之前定义的方法
//_syscall2(long,my_syscall,struct timespec*,x_time,struct timeval*,theTime)
//2.6.X系统之后定义的方法


long mysyscall(struct timeval *v_time,struct timespec *s_time){
	return syscall(338,v_time,s_time);
}


long do_pfcount(struct timeval *tv)
{
	return syscall(__NR_do_pfcount,tv);
}


int main(int argc,char **argv)
{
	/*
	struct 	timespec *ts;
	struct	timeval *tv;
	*/

	struct timespec ts;
	struct timeval tv;
	/*printf("jiffies:%ld\n",mysyscall(&tv,&ts));*/
	/*printf("pfcount:%ld\n",do_pfcount(&tv));*/
	/*printf("pid:%u\n",syscall(338,tv,ts));*/
	
	long pf_count = syscall(__NR_do_pfcount,&tv);
	long jiffies = syscall(__NR_mysyscall,&tv,&ts);
	printf("[%u.%u]:pfcount = %u\n",tv.tv_sec,tv.tv_usec,pf_count);
	printf("[%u.%u]:jiffies = %u\n",tv.tv_sec,tv.tv_usec,jiffies);
	/*printf("time is %ldsec.%ldusec:\n",tv.tv_sec,tv.tv_usec);*/
	/*printf("jiffies is %ld\n",syscall(__NR_mysyscall,&tv,&ts));*/
	/*printf("time is %ldsec.%ldusec\n",tv.tv_sec,tv.tv_usec);*/
	return 0;
}
