/*************************************************
	mytimer.c -description
	----------------------
copyright	:(C) 2002 by wolf
Function	:测试并发进程执行中的各种时间。
		 给定3个斐波纳契数值可选在36-45之间
**************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

/*static void psig_real(void);//父进程的3个定时中断处理函数原型*/
/*static void psig_virtual(void);*/
/*static void psig_prof(void);*/

static void psig_real(int sig);//父进程的3个定时中断处理函数原型
static void psig_virtual(int sig);
static void psig_prof(int sig);

static void c1sig_real(int sig);//子进程1的3个定时中断处理函数原型
static void c1sig_virtual(int sig);
static void c1sig_prof(int sig);

static void c2sig_real(int sig);//子进程2的3个定时中断处理函数原型
static void c2sig_virtual(int sig);
static void c2sig_prof(int sig);

long unsigned int fibonacci(unsigned int n);

//记录3种定时的秒数的变量
static long p_real_secs=0,c1_real_secs=0,c2_real_secs=0;
static long p_virtual_secs=0,c1_virtual_secs=0,c2_virtual_secs=0;
static long p_prof_secs=0,c1_prof_secs=0,c2_prof_secs=0;

//记录3种定时的毫秒秒数的结构变量
static struct itimerval p_realt,c1_realt,c2_realt;
static struct itimerval p_virtt,c1_virtt,c2_virtt;
static struct itimerval p_proft,c1_proft,c2_proft;


int count_real=0;
int count_virt=0;
int count_prof=0;
int main(int argc,char **argv)
{
	long unsigned fib = 0;
	int pid1,pid2;
	unsigned int fibarg;
	int status;
	int i;


	if (argc < 3){
		printf("Usage:testsig arg1 arg2 arg3\n");
		return 1;
	}

	//父进程设置3种定时处理入口
	signal(SIGALRM,psig_real);
	signal(SIGVTALRM,psig_virtual);
	signal(SIGPROF,psig_prof);

	//初始化父进程3种时间定时器
	p_realt.it_interval.tv_sec=9;
	p_realt.it_interval.tv_usec=999999;
	p_realt.it_value.tv_sec=9;
	p_realt.it_value.tv_usec=999999;
	setitimer(ITIMER_REAL,&p_realt,NULL);

	p_virtt.it_interval.tv_sec=9;
	p_virtt.it_interval.tv_usec=999999;
	p_virtt.it_value.tv_sec=9;
	p_virtt.it_value.tv_usec=999999;
	setitimer(ITIMER_VIRTUAL,&p_virtt,NULL);

	p_proft.it_interval.tv_sec=9;
	p_proft.it_interval.tv_usec=999999;
	p_proft.it_value.tv_sec=9;
	p_proft.it_value.tv_usec=999999;
	setitimer(ITIMER_PROF,&p_proft,NULL);
	//初始化定时器END

	pid1 = fork();
	//NOTE:fork不会继承itimer,execve会
	if (pid1 == 0){
		//子进程1设置3种定时处理入口
		signal(SIGALRM,c1sig_real);
		signal(SIGVTALRM,c1sig_virtual);
		signal(SIGPROF,c1sig_prof);

		//?:为什么要设置成999999呢?
		c1_realt.it_interval.tv_sec=9;
		c1_realt.it_interval.tv_usec=999999;
		c1_realt.it_value.tv_sec=9;
		c1_realt.it_value.tv_usec=999999;
		setitimer(ITIMER_REAL,&c1_realt,NULL);

		c1_virtt.it_interval.tv_sec=9;
		c1_virtt.it_interval.tv_usec=999999;
		c1_virtt.it_value.tv_sec=9;
		c1_virtt.it_value.tv_usec=999999;
		setitimer(ITIMER_VIRTUAL,&c1_virtt,NULL);

		c1_proft.it_interval.tv_sec=9;
		c1_proft.it_interval.tv_usec=999999;
		c1_proft.it_value.tv_sec=9;
		c1_proft.it_value.tv_usec=999999;
		setitimer(ITIMER_PROF,&c1_proft,NULL);

		//子进程1开始计算fib
		fib = fibonacci(atoi(argv[1]));

		//打印子进程1所花费的3种时间值
		getitimer(ITIMER_REAL,&c1_realt);
		printf("Child[1] fib=%ld\nChild[1] Real Time=%ldSec:%ldusec\n",fib,c1_real_secs+9-c1_realt.it_value.tv_sec,
				999999 - c1_realt.it_value.tv_usec);

		getitimer(ITIMER_VIRTUAL,&c1_virtt);
		printf("Child[1] Virtual Time=%ldSec:%ldusec\n",c1_virtual_secs+9-c1_virtt.it_value.tv_sec,
				999999 - c1_virtt.it_value.tv_usec);

		getitimer(ITIMER_PROF,&c1_proft);
		printf("Child[1] Prof Time=%ldSec:%ldusec\n",c1_prof_secs+9-c1_proft.it_value.tv_sec,
				999999 - c1_proft.it_value.tv_usec);
		//....
	}else if ( (pid2 = fork()) == 0){
		//子进程2设置3中定时中断入口
		//初始化子进程2的3中时间定时器

		signal(SIGALRM,c2sig_real);
		signal(SIGVTALRM,c2sig_virtual);
		signal(SIGPROF,c2sig_prof);

		c2_realt.it_interval.tv_sec=9;
		c2_realt.it_interval.tv_usec=999999;
		c2_realt.it_value.tv_sec=9;
		c2_realt.it_value.tv_usec=999999;
		setitimer(ITIMER_REAL,&c2_realt,NULL);

		c2_virtt.it_interval.tv_sec=9;
		c2_virtt.it_interval.tv_usec=999999;
		c2_virtt.it_value.tv_sec=9;
		c2_virtt.it_value.tv_usec=999999;
		setitimer(ITIMER_VIRTUAL,&c2_virtt,NULL);

		c2_proft.it_interval.tv_sec=9;
		c2_proft.it_interval.tv_usec=999999;
		c2_proft.it_value.tv_sec=9;
		c2_proft.it_value.tv_usec=999999;
		setitimer(ITIMER_PROF,&c2_proft,NULL);

		//子进程2开始计算fib
		fib = fibonacci(atoi(argv[2]));

		//打印子进程2所花费的3种时间值
		getitimer(ITIMER_REAL,&c2_realt);
		printf("Child[2] fib=%ld\nChild[2] Real Time=%ldSec:%ldusec\n",fib,c2_real_secs+9-c2_realt.it_value.tv_sec,
				999999 - c2_realt.it_value.tv_usec);

		getitimer(ITIMER_VIRTUAL,&c2_virtt);
		printf("child[2] virtual time=%ldsec:%ldusec\n",c2_virtual_secs+9-c2_virtt.it_value.tv_sec,
				999999 - c2_virtt.it_value.tv_usec);

		getitimer(ITIMER_PROF,&c2_proft);
		printf("Child[2] Prof Time=%ldSec:%ldusec\n",c2_prof_secs+9-c2_proft.it_value.tv_sec,
				999999 - c2_proft.it_value.tv_usec);

	}
	else{
		fib = fibonacci(atoi(argv[3]));
		printf("Parent fib=%ld\n",fib);
		getitimer(ITIMER_REAL,&p_realt);
		printf("Parent Real Time=%ldSec:%ldusec\n",p_real_secs+9-p_realt.it_value.tv_sec,
				999999 - p_realt.it_value.tv_usec);

		getitimer(ITIMER_VIRTUAL,&p_virtt);
		printf("Parent virtual time=%ldsec:%ldusec\n",p_virtual_secs+9-p_virtt.it_value.tv_sec,
				999999 - p_virtt.it_value.tv_usec);


		/*getitimer(ITIMER_PROF,&p_proft);*/
		/*printf("Parent Prof Time=%ldSec:%ldusec\n",p_prof_secs+9-p_proft.it_value.tv_sec,*/
		/*999999 - p_proft.it_value.tv_usec);*/
		getitimer(ITIMER_PROF,&p_proft);
		printf("Parent Prof Time=%ldSec:%ldusec\n",p_prof_secs+9-p_proft.it_value.tv_sec,
				999999 - p_proft.it_value.tv_usec);

		waitpid(0,&status,0);
		waitpid(0,&status,0);
	}

	return 0;
}


//父进程的3个定时中断处理函数
static void psig_real(int sig)
{
	printf("Parent real timer超时<%d>\n",++count_real);
	p_real_secs += 10;
	//有必要这样吗，给该计时器设置的值又不变。。。
	//p_realt.it_interval.tv_sec = 9;
	//p_realt.it_interval.tv_usec = 999999;
	//p_realt.it_value.tv_sec = 9;
	//p_realt.it_value.tv_usec = 999999;
	//setitimer(ITIMER_REAL,&p_realt,NULL);
}
static void psig_virtual(int sig)
{
	printf("Parent real timer超时<%d>\n",++count_virt);
	p_virtual_secs += 10;
}

static void psig_prof(int sig)
{
	printf("Parent real timer超时<%d>\n",++count_prof);
	p_prof_secs += 10;
}

//TODO:子进程1的3个定时中断处理函数
static void c1sig_real(int sig)
{
	printf("Child[1]real timer超时<%d>\n",++count_real);
	c1_real_secs += 10;
}
static void c1sig_virtual(int sig)
{
	printf("Child[1]virtual timer超时<%d>\n",++count_virt);
	c1_virtual_secs += 10;
}
static void c1sig_prof(int sig)
{
	printf("Child[1]prof timer超时<%d>\n",++count_prof);
	c1_prof_secs +=10;
}

//TODO:子进程2的3个定时中断处理函数
static void c2sig_real(int sig)
{
	printf("Child[2]real timer超时<%d>\n",++count_real);
	c2_real_secs += 10;
}
static void c2sig_virtual(int sig)
{	
	printf("Child[2]virtual timer超时<%d>\n",++count_virt);
	c2_virtual_secs += 10;
}
static void c2sig_prof(int sig)
{
	c2_prof_secs +=10;
	printf("Child[2]prof timer超时<%d>\n",++count_prof);
}

//fib的递归计算函数
long unsigned int fibonacci(unsigned int n)
{
	unsigned int f1=0,f2=1;
	long unsigned int f=0;
	unsigned int i;
/*
	for (i=0;i<n;i++){
		if ( i > 1){
			f =  f1 + f2;
			f1 = f2;
			f2 = f;
		}else{
			f = i; //0 or 1
		}
	}

*/

	if (n == 0 || n == 1){
		return n;
	}
	else {
		return fibonacci(n-1) + fibonacci(n-2);
	}
//	int sec ;
//	int bogomips = 4390;
//	int cpu_nr = 2;  
//	for (sec = 0;sec < 30;sec++){ //nearly 30 secs
//		for (i=0;i< bogomips* cpu_nr * 1000000;i++)
//		{ /*1 sec do nothing*/ }
//	}



}

