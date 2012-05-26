#include "stdio.h"
#include "/home/wolf/linux/linux-2.6.35.14/include/asm-generic/unistd.h"

static int errno;
_syscall0(long,HLKW);

int main(int argc,char **argv)
{
	printf("macro,jiffies:%u\n",HLKW());
	return 0;
}
