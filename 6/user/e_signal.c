#include <stdio.h>
#include <stdlib.h>
#include "/lib/modules/2.6.35.14/build/arch/x86/include/asm/unistd_32.h"
//其中的x.x.x是带有以上系统调用的内核版本号
int myevent_signal(int num){
return syscall(__NR_myevent_signal,num);
}
int main(int argc, char ** argv)
{
int i;
if(argc != 2)
{
printf("Usage: signal 2 | 4 | 6 | 2i ...\n");
return -1;
}
i = myevent_signal(atoi(argv[1]));
if(i)
printf("Wakeup event %d .\n",i);
else
printf("No event %d !\n",atoi(argv[1]));
return 0 ;
}

