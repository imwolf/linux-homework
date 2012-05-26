#define __KERNEL__
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "/lib/modules/2.6.35.14/build/arch/x86/include/asm/unistd_32.h"
#include <sys/syscall.h>
//其中的x.x.x是带有以上系统调用的内核版本号
int myevent_open(int num){
	printf("__NR_myevent_open:%d\n",__NR_myevent_open);
	return syscall(__NR_myevent_open,num);
}
int main(int argc, char ** argv)
{
	int i;
	if(argc != 2)
	{
		printf("Usage: open 2| 4 | 6 \n");
		return -1;
	}
	i = myevent_open(atoi(argv[1]));
	perror("myevent_open");
	/*printf("errno%d\n",errno);*/
	/*if(i)*/
		/*printf("Registed event %d \n",i);*/
	/*else*/
		/*printf("Registed event fail!\n");*/

	/*printf("NR_syscalls:%d\n",NR_syscalls);*/
	struct timeval tv;
	struct timespec ts;
	/*printf("%ld\n",syscall(339,&tv));*/
	/*printf("%d\n",syscall(339,&tv));*/
	printf("%d\n",syscall(348));
	return 0 ;
}
