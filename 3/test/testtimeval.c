#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

int handle_count=0;
void set_time(void)
{
	struct itimerval itv;
	itv.it_interval.tv_sec=10;//interval
	itv.it_interval.tv_usec=0;
	itv.it_value.tv_sec=0; //current
	itv.it_value.tv_usec=0;
	setitimer(ITIMER_REAL,&itv,NULL);
}

void alarm_handle(int sig)
{
	handle_count++;
	printf("have handle count is %d\n",handle_count);
}

void main(void)
{
	struct itimerval itv;
	signal(SIGALRM,alarm_handle);
	set_time();

	while(1){
		getitimer(ITIMER_REAL,&itv);
		printf("pass second is %d\n",(int)itv.it_value.tv_sec);
		sleep(1);
	}

	return;
}

