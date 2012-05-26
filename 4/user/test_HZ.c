#include <unistd.h>
#include <stdio.h>
#include <sys/param.h>

int main ()
{
	printf("%ld\n",sysconf(_SC_CLK_TCK));
	printf("%d\n",HZ);
}
