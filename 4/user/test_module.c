#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/param.h>

#define path "/proc/pm1dir/pm1file"
#define prefix ""
#define prelen 6
#define ITERNUM 10 //默认循环次数
int main(int argc,char * argv[])
{
	int n;
	if (argc >1 ){
		n = atoi(argv[1]);
	}else {
		n = ITERNUM;
	}

	FILE * fp = 0;
	char  buf[16];

	struct timeval tv;
	for ( ; n > 0 ; --n){
		fp = fopen(path,"r");
		printf("%s",fgets(buf,16,fp));
		fclose(fp);
		gettimeofday(&tv,NULL);
		printf("%10li.%06li\n***************************\n",tv.tv_sec,tv.tv_usec);
	}
}
