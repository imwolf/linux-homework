#include <stdio.h>
#include <sys/time.h>  //timeval,itimer等
#include <signal.h>
#include <unistd.h>

/**************************************
 * FUNCTION:mygettimeofday(struct timeval *tv)
 * DESCRIPTION:利用进程实时定时器ITIMER_REAL，每秒产生一个信号，并且计算已经经过的秒数
 * *************************************/
static int passed_sec = 0;
static int count = 0;
static int passed_usec = 0;
static void handle(int sig){
	if ( ++count == 100000){
		++passed_sec;	
		count = 0;
		passed_usec =0;
		kill(getppid(),SIGALRM);
		printf("passed:%08d\n",passed_sec);
	}else{
		passed_usec++;
	}
	/*printf("passed:%08d\n",passed_sec);*/
}

void mygettimeofday(){
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
