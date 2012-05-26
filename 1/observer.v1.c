#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#define LB_SIZE 80
enum TYPE{STANDARD,SHORT,LONG};

FILE * thisProcFile; 	//Proc打开文件指针,每次用它打开文件
struct timeval now;  	//系统时间日期
enum TYPE reportType;	//观察报告类型
char repTypeName[16];
char *lineBuf;		//proc文件读出行缓冲
int interval;		//系统负荷检测时间间隔
int duration;		//系统负荷检测时段
int iteration;
char c1,c2;		//字符处理单元

void sampleLoadAvg(){	//TODO:观察系统负荷
	int i = 0;
	char * avgs[6];
	char buf[16]={0};
	//读出、处理读出行，去除前导空格和无用空格 !有吗?
	thisProcFile = fopen("/proc/loadavg","r");
	if (thisProcFile == NULL){
		perror("cannot open file /proc/loadavg");
		return;
	}
	fgets(lineBuf,LB_SIZE + 1,thisProcFile);
	avgs[0] = strtok(lineBuf," ");
	for (i = 1;i<=5;++i){
		avgs[i] = strtok(NULL," /");
	}
	printf("前一分钟\t:\t%s\n",avgs[0]);
	printf("前五分钟\t:\t%s\n",avgs[1]);
	printf("前十五分钟\t:\t%s\n",avgs[2]);
	fclose(thisProcFile);
}

void sampleTime(){  //TODO:
	long uptime,idletime;
	int day,hour,minute,second;
	int i,j;
	char temp[80];
	i = j = 0;

	gettimeofday(&now,NULL);
	printf("当前时间\t:\t%s",ctime(&(now.tv_sec)));

	//打开计时文件
	thisProcFile = fopen("/proc/uptime","r");
	if (thisProcFile == NULL){
		perror("cannot open file /proc/uptime");
		return ;
	}
	//读出处理
	if (fgets(lineBuf,LB_SIZE + 1,thisProcFile) == NULL){
		perror("Read /proc/uptime(can't be empty) error");
		return ;
	}
	fclose(thisProcFile);	

	//分割字段，处理为可阅读格式
	char * token = strtok(lineBuf," ");
	uptime = atol(token);
	token = strtok(NULL," ");
	idletime = atol(token);

	//打印处理好的信息内容
	printf("uptime  \t:\t%ld seconds ago.\n",uptime);
	printf("idletime\t:\t%ld seconds.\n",idletime);
	//将启动时间的秒数转换为长整数
	struct tm *tm_uptime;
	time_t diff = now.tv_sec - uptime;
	tm_uptime = localtime(&diff);
	//转换成日时钟秒
	day = uptime / 86400;
	hour = ( uptime % 86400) / 3600;
	minute = ( uptime % 3600) / 60;
	second = uptime % 60;
	printf("系统已运行\t:\t%d天%d时%d分%d秒,",
			day,hour,minute,second);
	printf("于%d年%d月%d日 %02d:%02d:%02d启动\n",
			tm_uptime->tm_year + 1900,
			tm_uptime->tm_mon+1,tm_uptime->tm_mday,
			tm_uptime->tm_hour,tm_uptime->tm_min,
			tm_uptime->tm_sec);
	day = idletime / 86400;
	hour = ( idletime % 86400) / 3600;
	minute = ( idletime % 3600) / 60;
	second = idletime % 60;
	printf("空闲进程已运行\t:\t%d天%d时%d分%d秒\n",
			day,hour,minute,second);
}

int main(int argc,char *argv[])
{
	lineBuf = (char *)malloc(LB_SIZE + 1);
	reportType = STANDARD;
	strcpy(repTypeName,"Standard");
	
	if (argc == 1){ //观察部分A
		printf("\n******* PART A **********\n");
		reportType = SHORT;
		strcpy(repTypeName,"Short");
		//DONE:读出并显示系统当前时间
		struct tm *ts;
		time_t now;
		now = time(NULL);
		ts = localtime(&now);
		printf("[%04d年 %02d月 %02d日,%02d:%02d:%02d]\n",1900 + ts->tm_year,ts->tm_mon + 1,ts->tm_mday + 1,
				ts->tm_hour,ts->tm_min,ts->tm_sec);
		//或者读取/proc/driver/rtc文件下的内容
		//来查看实时时钟的内容 real-time clock
		printf("Real Time Clock\n");
		thisProcFile = fopen("/proc/driver/rtc","r");
		while( fgets(lineBuf,LB_SIZE + 1,thisProcFile)){
			if (strncmp(lineBuf,"rtc_time ",8) == 0){
				printf("时间%s",lineBuf+8);
			}
			if (strncmp(lineBuf,"rtc_date",8) == 0){
				printf("日期%s",lineBuf+8);
			}
		}
		//DONE:读出并显示机器名
		if ((thisProcFile = fopen("/proc/sys/kernel/hostname","r")) == NULL){
			perror("open file error:/proc/sys/kernel/hostname");
			return -1;
		}
		printf("\n---------- 机器名 HOSTNAME ----------\n");
		while (fgets(lineBuf,LB_SIZE + 1,thisProcFile) != NULL){
			printf("%s",lineBuf);
		}	
		fclose(thisProcFile);

		//DONE：读出并显示全部CPU信息
		thisProcFile = fopen("/proc/cpuinfo","r");
		if (thisProcFile == NULL){
			perror("open file error!");
			return -1;
		}
		printf("\n---------- CPU信息 ----------\n");
		int cpu_count = 1;
		while (fgets(lineBuf,LB_SIZE + 1,thisProcFile) != NULL){
			//如果只打印cpu类型
			if ( strncmp(lineBuf,"model name",10) == 0){
				printf("CPU%d-> ",cpu_count++);
				printf("%s",lineBuf);
			}
		}
		fclose(thisProcFile);	

		//DONE:读出并显示系统版本信息 
		printf("\n---------- 内核版本信息 ----------\n");
		thisProcFile = fopen("/proc/version","r");
		while (fgets(lineBuf,LB_SIZE + 1,thisProcFile) != NULL){
			printf("%s",lineBuf);
		}
		printf("\n---------- version_signature ----------\n");
		thisProcFile = freopen("/proc/version_signature","r",thisProcFile);
		while (fgets(lineBuf,LB_SIZE + 1,thisProcFile) != NULL){
			printf("%s",lineBuf);
		}

		printf("\n\n");
		fclose(thisProcFile);
		return 0;
	}

	if (argc > 1){
		sscanf(argv[1],"%c%c",&c1,&c2);
		if (c1 != '-'){
			//TODO
			printf("Usage:...");
			exit(1);
		}
	}
	if (c2 == 'b') { //观察部分B
		printf("**zo******** PART B **********\n");
		printf("--------- 内存信息 ----------\n");
		thisProcFile = fopen("/proc/meminfo","r");
		if (thisProcFile == NULL){
			perror("cannot open /proc/meminfo");
			return -1;
		}
		//读出文件全部内容
		//处理并显示
		while (fgets(lineBuf,LB_SIZE +1,thisProcFile) != NULL){
			if( strncmp(lineBuf,"MemTotal",8) == 0){
				printf("内存总量\t:%30.30s",lineBuf + 8 + 1);
			}
			if (strncmp(lineBuf,"MemFree",7) == 0){
				printf("空闲内存\t:%30.30s",lineBuf + 7 + 1);
			}
			if (strncmp(lineBuf,"Buffers",7) == 0){
				printf("已缓存(Buffers)\t:%30.30s",lineBuf + 7 + 1);
			}
			if (strncmp(lineBuf,"Cached",6 ) == 0){
				printf("高速缓存\t:%30.30s",lineBuf + 6 + 1);
			}
		}

		fclose(thisProcFile);

		//观察系统启动时间
		printf("---------- 启动时间 ----------\n");
		sampleTime();
	}
	else if (c2 == 'c'){ //观察部分C
		printf("*******PART C*********\n");
		//TODO:打开系统状态信息文件
		//读出全部内容
		//处理并显示
		if ( (thisProcFile = fopen("/proc/stat","r")) == NULL){
			perror("cannot open /proc/stat");
			return -1;
		}
		int i = 0;
		int cpu_count = -1;
		char * tokens[16];
		char * table_head[16] = {"CPU","用户态","用户低","系统态",
			"空闲态"};

		while (fgets(lineBuf,LB_SIZE + 1,thisProcFile) !=NULL){
			if (strncmp(lineBuf,"cpu",3) == 0){
				tokens[0] = strtok(lineBuf," ");
				for ( i=1; (tokens[i]=strtok(NULL," "))!=NULL;++i);
				if (++cpu_count == 0){
					printf("--------- CPU统计 ---------\n");
					printf("%s",table_head[0]);
					for (i = 1;i<= 4;++i)
						printf("\t|%10s",table_head[i]);
					printf("\n");
					printf("CPU总计");
				}
				else{
					printf("CPU%d",cpu_count);
				}
				for (i = 1;i <= 4; ++i){
					printf("\t|%10s",tokens[i]);
				}
				printf("\n");
				continue;
			}

			if (strncmp(lineBuf,"ctxt",4) == 0){
				printf("\n共进行了 %d 次上下文切换\n",atoi(lineBuf + 4));
				continue;
			}

			if (strncmp(lineBuf,"processes",9) == 0){
				printf("共进行了 %d 次fork\n",atoi(lineBuf + 9));
				continue;
			}

			if (strncmp(lineBuf,"procs_running",13) == 0){
				printf("%d 个进程正在运行\n",atoi(lineBuf + 13));
				continue;
			}
			if (strncmp(lineBuf,"procs_blocked",13) == 0){
				printf("%d 个进程阻塞中\n", atoi(lineBuf + 13));
				continue;
			}


		}

		fclose(thisProcFile);
	}
	else if (c2 == 'd'){ //观察部分D
		if ( argc < 4 ){
			printf("Usage:observer [-b] [-c] [-d int dur]\n");
			exit(1);
		}
		reportType = LONG;
		strcpy(repTypeName,"Long");

		printf("*******PART C*********\n");
		//用命令行参数指定的时间段和时间间隔
		interval = atoi(argv[2]);
		duration = atoi(argv[3]);
		//连续地读出系统负荷文件的内容用方便阅读的格式显示
		printf("---------- 系统负荷 ----------\n");
		sampleLoadAvg();

		interval = atoi(argv[2]);//时间间隔
		duration = atoi(argv[3]);//时间段
		iteration = 0;
		while (iteration < duration){
			sleep(interval);
			sampleLoadAvg();
			iteration = iteration + interval;
		}
		/*   while (1){
		     fgets(lineBuf,LB_SIZE + 1,thisProcFile);
		     rewind(thisProcFile);
		     printf("%s",lineBuf);
		     if (--interation > 0){
		     sleep(interval);
		     } else {
		     break;
		     }
		     }*/

	}	
}

