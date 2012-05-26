#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define LS 80

int main(){
	char *lineBuf,*tempBuf,*filename;
	char **argv1,**argv2;
	//pipe fd and pipe pid
	int fd[2];
	pid_t pid1,pid2;

	int argc,statuc,in_arg;
	int i,j,k;
	int f_out,f_in;
	char is_pip,is_dir,is_bkg;

	//分配命令行缓冲区
	//分配命令行参数缓区
	printf("$");
	//从标准输入读入一行
	
	//循环处理命令行参数分割符并统计出参数个数
	
	//分配参数串指针空间
	
	//从命令行中分割各个参数为单独的串
	
	//判断参数串中输入了哪些shell控制符号
	    //有管道符?
	       //分出管道两端命令名

		//建立管道
	    
	    //有I/O重定向符？
	    // 分出重定向文件名

	    //父进程建立子进程1
	    //如果为父进程
	      //有后台执行符?
	         //无，执行则等待子进程

	         //有，则立即shell准备好提示符

		//有管道，管道写入文件结束符
		close(fd[1]);//关闭管道输出
	    
	    //如果为子进程1
	      	if(is_pip == '|'）{ //有管道
				//关闭读管道
				//标准输出转向写管道
				close(fd[1]);
		}
		if (is_dir == '>'){//重定向输出，则打开该文件
			//代替标准输出
		}
		if (is_dir == '<‘){
		}
		//装入并执行子进程1新命令
		//
		if (is_pip == '|'){//有管道
		//父进程建立子进程2
		//....
		//

}
