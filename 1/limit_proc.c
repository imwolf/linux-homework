#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LB_SIZE 512

FILE *pid_max_file;
FILE *mem_file;
int DEFAULT_PM;    //default pid_max 
int current_p_no;  //current_pid_no
char *lineBuf;
long mem_limit;    //可用内存阈值

int main(int argc,char **argv)
{
	lineBuf = (char*)malloc(LB_SIZE );
	//打开文件并且记住默认pid_max值
	pid_max_file=fopen("/proc/sys/kernel","r");
	if (pid_max_file == NULL){
		perror("cannot open file\n");
		return -1;
	}

	fgets(lineBuf,LB_SIZE,pid_max_file) != NULL ;
	DEFAULT_PM = atol(lineBuf);
	
	//循环检测内存可用内存数
	//小于等于阈值时修改到pid_max_file中
	//大于时维持为系统默认值
	

}

