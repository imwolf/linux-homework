%option noyywrap nodefault


%{
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
//#define DEBUG

extern char *line_buf;
extern int line_len;


static int my_yyinput(char *yybuf,int max_size);


//处理过程中就把参数给清了，所以没有必要传入参数数目
static int do_list_cmd();  /*list-1个？或多个管道，;&  ||  &&分割*/
static int do_pipe_cmd(/*int argc,*/char **pipeline,int bg);/*管道一个或多个命令，|分割*/
static char** scan_redirect(char **oldcmd);
static int do_simple_cmd(/*int argc,*/char **cmd,int prefd[],int postfd[],int bg);


static void add_token(const char *token);
static char *expand(const char *token);


//------------内置命令-------------------------
typedef int (*buildin_cmd_handle) (char **);
static buildin_cmd_handle get_cmd_handle(const char *cmd);
static int do_exit(char **);
static int do_echo(char **);
static int do_cd(char **);
static int do_export(char **);

typedef struct
{
	const char *cmd;
	buildin_cmd_handle handle;
}CMD_ENTRY;

const CMD_ENTRY buildin_cmd_table[] = 
{
	{"exit", 	do_exit},
	{"cd", 		do_cd},
	{"echo", 	do_echo},
	{"export", 	do_export},
	{0, 0}
};

/*由于输入源既不是终端也不是标准文件，而是readline返回的字符串，所以重定义该宏*/
#undef YY_INPUT
#define YY_INPUT(yybuf,result,max_size) (result = my_yyinput(yybuf,max_size))

#define TOKENSZ 200
char * tokens[TOKENSZ]; //读到的行分解之后存放的地方
int tok_cnt = 0;
char *pos_param[10];  //位置参量
int ARGC;  //FIXME
int EXIT_STATUS;
/*FIXME:注释*/
%}

%x QUOTE
%x SINGQUOTE
blank [ \t]

%%

";"	{add_token(yytext);}
">"	{add_token(yytext);}
"<"	{add_token(yytext);}
"|"	{add_token(yytext);}
"&"	{add_token(yytext);}
[^ \t\n|<>;&"]+	{add_token(expand(yytext)); /*可能的话，进行参数扩展之后再加入*/}
\n	{add_token(yytext);do_list_cmd(); return 0;} /*换行符也是一个控制字符,return 0要不要加?*/
.	{}

%%

static buildin_cmd_handle get_cmd_handle(const char *cmd)
{
	int i = 0;
	while(buildin_cmd_table[i].cmd){
		if (strcmp(buildin_cmd_table[i].cmd,cmd) == 0)
			return buildin_cmd_table[i].handle;
		i++;
	}
	return 0;
}

//FIXME:
static int do_exit(char **argv)
{
	int val = 0;
	free(argv[0]);
	argv[0] = 0;
	if (argv[1] != NULL){
		val = atoi(argv[1]);
		free(argv[1]);
		argv[1] = 0;
	}
	exit(val);
}

static int do_cd(char **argv)
{
	return 0;
}

static int do_export(char **argv)
{
	return 0;
}

static int do_echo (char **argv)
{
	int	i = 0;
	while (argv[++i]){
		printf("%s ",argv[i]);
	}
	printf("\n");
	return 0;
}

int *pipefds[200];  //存放产生的管道，每行命令结束后在Shell进程中将他们统一关闭
/**
* @brief	列表命令;以";" "&"结尾的一个或多个pipeline;
*
* @return	TODO?
*/
static int do_list_cmd()
{
	int i = 0;
	int j = 0; 
	int bg = 1;
	while(tokens[i]){
		#ifdef DEBUG
		printf("tokens[%d]:%s\n",i,tokens[i]);
		#endif
		
		/* FIXME:加入;<newline> 或者 &<newline>连在一起*/
		if ( !strcmp(tokens[i],";") || !strcmp(tokens[i],"\n")){
 			//<newline>是lists的可选结尾，行为与;一致，顺序执行
			free(tokens[i]);
			tokens[i] = 0;	//防止悬垂指针?
			if (j != i)  //排除;<newline>这样的情况
				do_pipe_cmd(tokens+j,!bg);
			j = ++i;
		}else if ( !strcmp(tokens[i],"&") ){
			//后台执行
			free(tokens[i]);
			tokens[i] = 0;
			if (j != i)
				do_pipe_cmd(tokens+j,bg);
			j = ++i;
		}else 	//准备读取下一个token
			++i;  
	}
	
	//token此时应该已经全部free了(数据不一定为0)
	//计数清零，准备下一次读入
	tok_cnt = 0;
	
	//TODO:返回什么类型?
	return 0;
}

/**
* @brief	运行pipeline
*
* @param	pipeline	char*数组；pipeline即以"|"分割的1个或多个简单命令
*
* @return	TODO?
*/
static int do_pipe_cmd(char **pipeline,int bg)
{
	int i = 0;	
	int j = 0;	/*简单命令在pipeline的偏移*/
	int is_first_cmd = 1; /*need not connect its stdin */
	int prefd[2];
	int postfd[2];

	while(pipeline[i] /*!= 0*/){
		if (strcmp(pipeline[i],"|" )== 0) { //pipeline由|或者&分割
			free(pipeline[i]);
			pipeline[i] = 0;
			
			pipe(postfd);
			
			if (!is_first_cmd)
				do_simple_cmd(pipeline+j,prefd,postfd,bg);
			else
				do_simple_cmd(pipeline+j,0,postfd,bg);

			is_first_cmd = 0;
			/*后一个命令的后管道作为后一个命令的前管道*/
			prefd[0] = postfd[0];
			prefd[1] = postfd[1];
			j = ++i;
		}//since we have a flag bg,there is no need for us to do background here,just pass it do simple cmd.
		else i++;
	}

	//pipeline中的最后一个cmd没有|或&结尾，在此处理
	if (!is_first_cmd)
		do_simple_cmd(pipeline+j,prefd,0,bg);
	else
		do_simple_cmd(pipeline+j,0,0,bg);
	
	//FIXME
	return 0;
}

/**
* @brief	从左至右扫描简单命令，按此顺序进行重定向；
*		注意：oldcmd将被置空
*
* @param	oldcmd	包含重定向的简单命令;是以NULL结尾的char*数组
*
* @return	去除重定向之后char*数组，以NULL结尾；用以执行exec命令
*/
static char** scan_redirect(char **oldcmd)
{
	//TODO
	char **newcmd = (char **)calloc(TOKENSZ+1 , sizeof(char *));	

	int i = 0; //oldcmd读指针
	int j = 0; //new_cmd写指针
	//从左至右顺序处理重定向
	while (oldcmd[i] != NULL){
		#ifdef DEBUG
		printf("%s\t",oldcmd[i]);
		fflush(stdout);
		#endif
		if (strcmp(oldcmd[i],"<") == 0){ //重定向输入

			close(STDIN_FILENO);
			if (!fopen(oldcmd[i+1],"r")){
				perror("cannot open in file\n");
				exit(-1);
			}
			free(oldcmd[i]);
			free(oldcmd[i+1]);
			oldcmd[i] = oldcmd[i+1] = 0;
			++i;
		}else if (strcmp(oldcmd[i],">") == 0){ //重定向输出

			close(STDOUT_FILENO);
			if (!fopen(oldcmd[i+1],"w")){
				perror("cannot open out file\n");
				exit(-1);
			}
			
			free(oldcmd[i]) ;
			free(oldcmd[i+1] );
			oldcmd[i] = oldcmd[i+1] = 0;
			++i;
		}else {
			newcmd[j] = (char *)malloc(sizeof(oldcmd[i]) + 1);
			strcpy(newcmd[j],oldcmd[i]); 
			
			free(oldcmd[i]);
			oldcmd[i] = 0;
			j++;
		}
		++i;
	}
	
	newcmd[j+1] = 0;
	#ifdef DEBUG
	i = 0;
	printf("newcmd:\n");
	while (newcmd[i]){
		printf("\t%s",newcmd[i++]);
	}
	printf("\n");
	#endif
	return newcmd;
}


/**
* @brief	运行简单命令
*
* @param	cmd		简单命令(包含重定向)
* @param	prefd[]		与前命令连接的管道		
* @param	postfd[]	与后命令连接的管道
* @param	bg		是否后台运行
*
* @return	
*/
static int do_simple_cmd(char **cmd,int prefd[],int postfd[],int bg)
{
	int pid;
	int status;
	
	//-----------------内部命令----------------------TODO:管道什么的
	buildin_cmd_handle hd;
	
	if ((hd = get_cmd_handle(cmd[0]))){
		if ( prefd ){
			if (dup2(prefd[0],STDIN_FILENO))
				perror("DUP prefd0\n");
			close(prefd[0]);//TODO?
		}

		if ( postfd ){		//子进程要向该管道写
			//close(postfd[0]);   //除了最后一个命令
			if (dup2(postfd[1],STDOUT_FILENO))
				perror("DUP postfd1\n");
			close(postfd[1]);
		}

		char **argv = scan_redirect(cmd);
		return (*hd)(argv);
	}	

	//--------外部命令----------------
	if ( (pid = fork()) < 0){
		perror("cannot fork!\n");
		exit(-1);
	}else 
	if (pid == 0){ //child
		//先做管道连接再做重定向
		#ifdef DEBUG
		printf("CHILD......\n");
		printf("prefd=%d,postfd=%d\n",prefd,postfd);
		#endif
		if ( prefd ){
			//close(prefd[1]);  //子进程要向从该管道读；
			//除了第一个命令，该管道的写入端其实已经在父进程关闭了(close(postfd[1]),而子进程只复制打开的描述符;
			//第一个命令没有prefd
			//简单来讲：对于非首尾命令，在执行的时候prefd一定只有读取的一端是开着的，写入的一端一定是关着的
			//对于第一个命令：关掉postfd读端，将标准输出重定向至管道写入端，关掉描述符，这些操作与后面除末命令行为一致
			//对于最后一个命令：重定向输入之后只需关掉管道的读取端
			if (dup2(prefd[0],STDIN_FILENO))
				perror("DUP prefd0\n");
			close(prefd[0]);//TODO?
		}

		if ( postfd ){		//子进程要向该管道写
			close(postfd[0]);   //除了最后一个命令
			if (dup2(postfd[1],STDOUT_FILENO))
				perror("DUP postfd1\n");
			close(postfd[1]);
		}
		
		//重定向
		char ** argv = scan_redirect(cmd);
		execvp(argv[0],argv);	//FIXME:argv是malloc出来的
		
		if (errno == 2){//FIXME?
			fprintf(stderr,"no such command:%s\n",argv[0]);
		}
		//perror("execvp error\n");  /*exec never reachs here*/
		exit(-1);
		
	}else{ //parent
		//TODO & background 
		#ifdef DEBUG
		printf("PARENT AFTER FORK\n");
		#endif
		if (!bg)
			waitpid(pid,&status,0);
		else 
			waitpid(pid,&status,WNOHANG); //后台命令，立即返回
		/*if (WIFEXITED(status)){
			EXIT_STATUS = WEXITSTATUS(status);
		}*/
		if (postfd) { /*在fork的时候父进程与子进程分别对该管道的输出端有引用，即该管道输出端引用计数为2；*/
				//不论是阻塞等待还是后台执行，父进程和子进程对该文件描述符的引用都必须关闭才能完全关闭该文件结构
				//而子进程在退出后应该是会自动关闭的?而父进程(Shell)中需要我们手动关闭。
			close(postfd[1]); //已完成一个命令，输出完毕;该管道的输出端已无用
					  //在父进程中必须把向后的管道写入端关闭，以表示输入结束(相当于输入EOF?)
					  //下一个命令执行(fork)的时候，该管道将只有读取端是打开的,子进程也只将这个管道读取端的描述符(而且被表示成了prefd)
		}

		if (prefd) {
			close(prefd[0]);
		}
		return 0;
	}
}

/**
* @brief	将词法分析器匹配到的yytext复制到tokens数组；tokens数组存储参数以备语法分析
*		注意:tokens数组中的元素是通过malloc分配到内存的，记得free
*
* @param	token	词法分析器匹配到的yytext或者经过$扩展之后的变量
*/
static void add_token(const char *token)
{	
	#ifdef DEBUG	
	printf("IN ADD_TOKEN._FILE:%s\tLINE:%d\n",__FILE__,__LINE__);
	#endif
	if (tok_cnt <= TOKENSZ){
		tokens[tok_cnt++] = (char*)malloc(strlen(token) + 1);
		strcpy(tokens[tok_cnt-1],token);
		tokens[tok_cnt] = 0; //make sure tokens is NULL terminated;
	}
	else{
		printf("too many tokens\n");
		exit(-1);
	}
}

/**
* @brief	拷贝flex匹配的yytext,同时做$扩展；
*
* @param	token	flex匹配到的字符串
*
* @return	token的原样拷贝或者做$扩展之后的拷贝
*/
char buf[TOKENSZ];
static char *expand(const char* token)
{
	int l = 0;
	//char buf[200];  //FIXME:Size ?
	#ifdef DEBUG
	printf("IN EXPAND!!!_FILE:%s\tLINENO:%d\n",__FILE__,__LINE__);
	#endif
	int len = strlen(token);
	int i;
	for ( i = 0;i<len;i++){
		if ('$' == token[i]){
			//TODO:补充
			if (token[i+1] == '$'){
			sprintf(buf+l,"%d",getpid());
			l = strlen(buf);
			i++;
			}else if (isdigit(token[i+1])){ //Positional Parmeter
				int t = token[i+1]-0x30;
				sprintf(buf+l,"%s",pos_param[t]);
				l = strlen(buf);
				i++;
			}else if (token[i+1] == '#'){
				sprintf(buf+l,"%d",ARGC );
				l = strlen(buf);
				i++;
			}else if(token[i+1] == '?'){
				sprintf(buf+l,"%d",EXIT_STATUS);
				l = strlen(buf);
				i++;
			}
			else{ //env
			}
		}else{
			buf[l] = token[i];
			l++;
		}
	}
	buf[l] = 0;   //make sure it's null terminated;
	
	return buf;
}

static int my_yyinput(char * yybuf,int max_size)
{
	//输入大于能够flex缓冲区能接受的大小？直接提示错误
	//16K还会溢出?而且错误的话应该返回什么值0表示EOF
	if ( line_len < max_size ){ 
		//memcpy(yybuf,line_buf,line_len); //还需添加尾部的NULL
		strcpy(yybuf,line_buf);
		#ifdef DEBUG
		printf("FILE：%s,LINE:%d\n",__FILE__,__LINE__);
		printf("contents in yybuf:%s\n",yybuf);
		printf("contents length :%d\n",strlen(yybuf));
		#endif
		return line_len;
	}
	else {
		printf("your input is too long\n");
		return -1;
	}
}
