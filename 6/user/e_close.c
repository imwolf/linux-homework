#include "/lib/modules/2.6.35.14/build/arch/x86/include/asm/unistd_32.h"
#include <stdio.h>
#include <stdlib.h>
int myevent_close(int flag){
return syscall(__NR_myevent_close,flag);
}
int main(int argc, char ** argv)
{
int i;
if(argc != 2)
return -1;
i = myevent_close(atoi(argv[1]));
printf("%d\n",i);
return 0 ;
}

