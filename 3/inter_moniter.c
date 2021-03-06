/********************************************
 *每隔1.25毫秒时间内，系统发生了多少次中断
 *
 ********************************************/

#include <stdio.h>
#include <sys/time.h>  //ITIMER_REAL,timeval等
#include <signal.h>
#include <string.h>    //strstr()
#include <stdlib.h>    //atoi()
#define INTERVAL_USEC 1250
long int_sum;
struct itimerval itvs[3];
void sigh_real(int sig);
void sigh_virt(int sig);
void sigh_prof(int sig);

long cur_intr = 0;
long old_intr_real;
long old_intr_virt;
long old_intr_prof;
FILE * procFile;
long read_intr(int caller);

//1.ITIMER_REAL;2.ITIMER_VIRTUAL;3.ITIMER_PROF
int map_timer(int which);
void init_itvs();

void clean(int sig);

long sec  = 0;
long usec = 1250;

#define DEBUG
#ifdef DEBUG
	FILE * fd_real,* fd_virt,* fd_prof;
#endif
int main (int argc,char **argv)
{
	if (argc > 1){
		if (argc != 3) {
			printf("usage:./inter_moniter sec usec\n");
			exit(1);
		}
		else{
			sec = atol(argv[1]);
			usec = atol(argv[2]);
		}

	}

	signal(SIGINT,clean);
	void (*handles[3])(int sig); //信号处理函数数组
	handles[map_timer(ITIMER_REAL)] = sigh_real;
	handles[map_timer(ITIMER_VIRTUAL)] = sigh_virt;
	handles[map_timer(ITIMER_PROF)] = sigh_prof;

	
#ifdef  DEBUG
	fd_real = fopen("/tmp/fd_real","w+");
	fd_virt = fopen("/tmp/fd_virt","w+");
	fd_prof = fopen("/tmp/fd_prof","w+");
#endif	
	if ( (procFile = fopen("/proc/stat","r")) == NULL){ //打开文件，为读取做准备
		perror("error open file /proc/stat");
		exit(1);
	}
	old_intr_real = old_intr_prof = old_intr_virt = cur_intr = read_intr(0);
	printf("初始中断数:%ld\n",cur_intr);
	//设置3种itimervals
	init_itvs();
	//设置3种定时器
	setitimer(ITIMER_REAL,&itvs[map_timer(ITIMER_REAL)],NULL);
	setitimer(ITIMER_VIRTUAL,&itvs[map_timer(ITIMER_VIRTUAL)],NULL);
	setitimer(ITIMER_PROF,&itvs[map_timer(ITIMER_PROF)],NULL);

	//设置3种信号处理函数
	signal(SIGALRM,handles[0]);
	signal(SIGVTALRM,handles[1]);
	signal(SIGPROF,handles[2]);

	while(1);
	return 0;
}

void clean(int sig)
{
	/*fclose(procFile);*/
	printf("Bye...\n");
	exit(1);
}

//FIXME:三个函数会不会并发执行?
//FIXME:使用同一个old_intr为什么全是0


void sigh_real(int sig)
{
	cur_intr = read_intr(0);
	if (cur_intr == -1) 
	{
		printf("real\n");
		exit(1);
	}
	printf("Real Timer，中断差为:%ld,cur:%ld,old:%ld\n",cur_intr-old_intr_real,cur_intr,old_intr_real);
	old_intr_real = cur_intr;
}

void sigh_virt(int sig)
{
	cur_intr = read_intr(1);
	if (cur_intr == -1) 
	{
		printf("virt\n");
		exit(1);
	}
	printf("Virtual Timer，中断差为:%ld,cur:%ld,old:%ld\n",cur_intr-old_intr_virt,cur_intr,old_intr_virt);
	old_intr_virt = cur_intr;
}

void sigh_prof(int sig)
{
	cur_intr = read_intr(2);
	if (cur_intr == -1) 
	{
		printf("prof\n");
		exit(1);
	}
	printf("Prof Timer，中断差为:%ld,cur:%ld,old:%ld\n",cur_intr-old_intr_prof,cur_intr,old_intr_prof);
	old_intr_prof = cur_intr;
}

int map_timer(int which)
{
	switch (which)
	{
		case ITIMER_REAL: 
				return 0;
				break;
		case ITIMER_VIRTUAL: 
				return 1;
			   	break;
		case ITIMER_PROF: 
				return 2;
		     		break;		  
	}
}

void init_itvs()
{
	int i=0;
	printf("sec=:%ld,usec=%ld\n",sec,usec);
	for (i=0;i<3;++i)
	{
		itvs[i].it_interval.tv_sec = sec;
		itvs[i].it_interval.tv_usec = usec; //间隔为1.25毫秒=1250微秒
		itvs[i].it_value.tv_sec = 0;
		itvs[i].it_value.tv_usec = 1250;
	}
}

char buf0[2048];
#ifdef DEBUG
char buf1[2048];
char buf2[2048];
char buf3[2048];
int c0,c1,c2;
#endif 
long read_intr(int caller)
{
	char *intr;
	char * buf = buf0;
	if ( (procFile = fopen("/proc/stat","r")) == NULL){ //打开文件，为读取做准备
		perror("error open file /proc/stat");
		exit(1);
	}

#ifdef DEBUG
		switch (caller){
			case 0:++c0;
				buf = buf1;
			       break;
			case 1:buf = buf2;
			       ++c1;
			       break;
			case 2:buf = buf3;
			       ++c2;
			       break;
		}
#endif

	//该函数要被多次调用，所以每次都将读写位置置到开始
//	rewind(procFile);//TODO:是不是这里有问题?
	//FIXME:按行读可能会出现行被被截断，从而读出的数字错误的问题
	//不过这里缓冲区够大，而且proc/stat文件每一行文件并不多，应该没问题
	while (fgets(buf,2048,procFile) != NULL){
#ifdef DEBUG
		switch (caller){
			case 0:fprintf(fd_real,"[%d] [%ld]",c0,ftell(procFile)-strlen(buf));
				fputs(buf,fd_real);
			       break;
			case 1:fprintf(fd_virt,"[%d] [%ld]",c1,ftell(procFile)-strlen(buf));
			       fputs(buf,fd_virt);
			       break;
			case 2:fprintf(fd_prof,"[%d] [%ld]",c2,ftell(procFile)-strlen(buf));
			       fputs(buf,fd_prof);
			       break;
		}
#endif

		/*if ( (intr = strstr(buf,"intr")) != NULL){*/
		if (strncmp(buf,"intr",4) == 0){
			intr = buf;
			//rewind(procFile);
			fseek(procFile,0,SEEK_SET);
			fclose(procFile);
			return atoi(intr+4+1);   //"intr "长度，intr起始即为总中断数			
		}
		/*else*/
			/*printf("%s",buf);*/
	}


#ifdef DEBUG
		switch (caller){
			case 0:fputs("\n\n===SHOULD NEVER BE HERE======\n",fd_real);
			       break;
			case 1:fputs("\n\n===SHOULD NEVER BE HERE======\n",fd_virt);
			       break;
			case 2:fputs("\n\n===SHOULD NEVER BE HERE======\n",fd_prof);
			       break;
		}
#endif
	/*printf("reading error,fail in finding string 'intr' in /proc/stat,try it again:%ld\n",read_intr(caller));*/
	return -1; //ERROR
}
