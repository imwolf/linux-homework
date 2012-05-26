#include <stdio.h>
#include <stdlib.h>
#include "/lib/modules/2.6.35.14/build/arch/x86/include/asm/unistd_32.h"
//其中的x.x.x是带有以上系统调用的内核版本号
int myevent_open(int num,int value){
	return syscall(__NR_myevent_open,num,value);
}
int main(int argc, char ** argv)
{
	int i;
	if(argc != 3)
	{
		printf("Usage: open 2 val | 4 val | 6 val\n");
		return -1;
	}
	i = myevent_open(atoi(argv[1]),atoi(argv[2]));
	if(i)
		printf("Registed event %d with value=%d.\n",i,atoi(argv[2]));
	else
		printf("Registed event fail!\n");
	return 0 ;
}
