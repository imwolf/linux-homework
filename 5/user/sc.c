#include "sys/time.h"
#include "stdio.h"
#include "/home/wolf/linux/linux-2.6.35.14/arch/x86/include/asm/unistd_32.h"
#define __NR_HLKW 294
int main(int argc,char **argv)
{
	printf("syscal,__NR_mysyscall:%u\n",__NR_mysyscall);
	printf("syscal,__NR_do_pfcount:%u\n",__NR_do_pfcount);
	printf("NR_syscalls:%u\n",NR_syscalls);

	struct timeval tv;
	struct timespec ts;
	printf("pfcount:%u\n",syscall(__NR_do_pfcount,&tv));
	printf("jiffies(mysyscall):%u\n",syscall(__NR_mysyscall,&tv,&ts));
	return(0);
}
