#include <stdio.h>
#include <sys/time.h>  //timeval,itimer等
#include <signal.h>
#include <unistd.h>
#include <time.h>

/**************************************
 * FUNCTION:mygettimeofday(struct timeval *tv)
 * DESCRIPTION:利用进程实时定时器ITIMER_REAL，每秒产生一个信号，并且计算已经经过的秒数
 * *************************************/
static int passed_sec = 0;
static int count = 0;
static int passed_usec = 0;
static struct timeval tv;
static void handle(int sig){
	if (tv.tv_usec == 999999){
		tv.tv_usec = 0;
		tv.tv_sec++;
	}else
		tv.tv_usec++;

	struct tm* ptm;
	ptm = localtime(&(tv.tv_sec));

	printf("现在时间 %02d月%02d日 %02d:%02d:%02d",ptm->tm_mon,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
	printf(":%06d\n",tv.tv_usec);
	
}

void mygettimeofday(){
	gettimeofday(&tv,NULL);

	pid_t pid;
	if ( (pid = fork()) == 0){ //子进程设好计时器之后一直循环
		struct itimerval it;
		it.it_interval.tv_sec = 0;
		it.it_interval.tv_usec = 1;
		it.it_value.tv_sec = 0;
		it.it_value.tv_usec = 1;
	
		setitimer(ITIMER_REAL,&it,NULL);

		signal(SIGALRM,handle);

		while (1) sleep(1);
	}else{//父进程直接继续下一循环
		return;
	}
}

static int main_sec=0;
void action_1s(int sig){
	main_sec++;
}

int main()
{
	int opt;
	mygettimeofday();//fork出子进程，设置定时器，信号

	signal(SIGALRM,action_1s);
	while (1) sleep(10);
}
