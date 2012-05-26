#include <stdio.h>
#include <stdlib.h>
#include "/lib/modules/2.6.35.14/build/arch/x86/include/asm/unistd_32.h"
//其中的x.x.x是带有以上系统调用的内核版本号
int myevent_wait(int num){
return syscall(__NR_myevent_wait,num);
}
int main(int argc, char ** argv)
{
int i;
if(argc != 2)
{
printf("Usage: wait 2 | 4 | 6 | 2i ...\n");
return -1;
}
printf("Event %d will sleeping !\n",atoi(argv[1]));
i = myevent_wait(atoi(argv[1]));
printf("Event %d wakeup !\n",i);
return 0 ;
}

