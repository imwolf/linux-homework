#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFSIZE 32
/*--------------------------------------
 * 功能：读取/修改sysrq和ctrl-alt-del的值
 * 目的：学习通过proc文件系统修改内核参数
 */
int readSysrq(void);
int readCad(void);
int write_sysrq(int value);
int write_cad(int value);


FILE *sysrqFile;
FILE *cadFile;
char *path1 = "/proc/sys/kernel/sysrq";
char *path2 = "/proc/sys/kernel/ctrl-alt-del";
char buf[BUFSIZE];	/*TODO:帮助信息改进*/
char * help_str = "Usage:observer [-rx] [-wx [value]]\n \
\t-r :read the value\n \
\t-w :write value to the file;\n \
\t    by default,the value is reversed\n \
\t x : s=sysrq, c=ctrl-alt-del\n";
int main(int argc,char **argv)
{
	if (argc == 1 || argc >= 4 )
	{
		printf(help_str,argv[0]);
		return -1;
	}
	int cad_value;
	int sysrq_value;
	char c1,c2,c3;
	sysrq_value = readSysrq();
	cad_value = readCad();
	sscanf(argv[1],"%c%c%c",&c1,&c2,&c3);
	if (c1 != '-'){
		printf("%s\n",help_str);
		return -1;
	} 
	//读取文件值
	if (c2 == 'r'){
		if ( c3 == 's'){
			printf("%s文件的内容是：\t%d\n",path1,sysrq_value);
		} else if (c3 == 'c'){
			printf("%s文件的内容是：\t%d\n",path2,cad_value);
		} else {
			printf(help_str,argv[0]);
		}
		return 0;
	}

	//修改文件值
	if (c2 == 'w'){
		int tmp = 0;
		if (argc == 3){
			if (argv[2][0] =='0' && argv[2][1] =='x'){
				tmp = strtol(argv[2],NULL,16);
			}
			else{
				tmp = strtol(argv[2],NULL,0);
			}
		}

		if (c3 == 's'){
			tmp = (argc==3)?tmp:0xff^sysrq_value;
			write_sysrq(tmp);
		}else if (c3 == 'c'){
			tmp = (argc==3)?tmp:0xff^cad_value;
			write_cad(tmp);
		}

		return 0;

	}
}

int readSysrq(){
	//处理sysrqFile
	sysrqFile = fopen(path1,"r+");
	if (sysrqFile == NULL)
	{
		perror("cannot open file\n ");
		exit(-1);
	}
	fgets(buf,BUFSIZE,sysrqFile);
	/*if (fputs("1",sysrqFile) == EOF){
	  perror("wrong?\n");
	  return -1;
	  }*/

	fclose(sysrqFile);
	return atoi(buf);
}

int readCad(){
	//处理ctrl-alt-del文件
	cadFile = fopen(path2,"r+");
	if (cadFile == NULL)
	{
		perror("cannot open file\n ");
		exit(-1);
	}
	fgets(buf,BUFSIZE,cadFile);
	/*printf("%s文件的内容是:\t%d\n",path2,cad_value);
	  if (fputs("323232",cadFile) == EOF){
	  perror("wrong?\n");
	  return -1;
	  }*/

	fclose(cadFile);
	return atoi(buf);
}

int write_sysrq(int value){
	sysrqFile = fopen(path1,"w");
	if (sysrqFile == NULL){
		perror("cannot open file1\n");
		return -1;
	}
	fprintf(sysrqFile,"%d",value);
	fclose(sysrqFile);
	return 0;
}
int write_cad(int value){
	cadFile = fopen(path2,"w");
	if (sysrqFile == NULL){
		perror("cannot open file1\n");
		return -1;
	}
	fprintf(cadFile,"%d",value);
	fclose(cadFile);
	return 0;
}
