%{
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


extern char buf[];
extern char* myptr;
extern char* mylim;

extern void display_history_list();
extern void history_finish();

static int my_yyinput(char* buf, int max);
static int file_exist(const char* file, char* buffer);
static void free_resource();

static int do_exit(int, char**);
static int do_export(int, char**);
static int do_echo(int, char**);
static int do_cd(int, char**);
static int do_history(int, char**);

static void add_arg(const char* xarg);
static void add_simple_arg(const char* xarg);
static void reset_args();
static void do_list_cmd();
static int do_pipe_cmd(int argc, char** argv);
static int do_simple_cmd(int argc, char** argv, int prefd[], int postfd[]);

#undef 	YY_INPUT
#define YY_INPUT(b, r, ms) (r = my_yyinput(b, ms))

char* argbuf[200];
int argcnt = 0;

typedef int (*buildin_cmd_handle)(int, char**);
typedef struct
{
	const char* cmd;
	buildin_cmd_handle handle;
} CMD_ENTRY;

const CMD_ENTRY buildin_cmd_table[] = 
{
	{"exit", 	do_exit},
	{"cd", 		do_cd},
	{"echo", 	do_echo},
	{"export", 	do_export},
	{"history", do_history},
	{0, 0}
};

%}

%x		QUOTE
%x		SINGQUOTE
blank	[ \t]

%%

"\""			{BEGIN QUOTE;}
<QUOTE>[^\n"]+	{add_arg(yytext);}
<QUOTE>"\""		{BEGIN 0;}
<QUOTE>\n		{BEGIN 0; do_list_cmd(); reset_args();}
";"				{add_simple_arg(yytext);}
">"				{add_simple_arg(yytext);}
"<"				{add_simple_arg(yytext);}
"|"				{add_simple_arg(yytext);}
[^ \t\n|<>;"]+	{add_arg(yytext);}
\n				{do_list_cmd(); reset_args();}
.				;

%%

static buildin_cmd_handle get_cmd_handle(const char* cmd)
{
	int i = 0;
	while(buildin_cmd_table[i].cmd) {
		if(strcmp(buildin_cmd_table[i].cmd, cmd) == 0)
			return buildin_cmd_table[i].handle;
		i++;
	}
	return 0;
}
static void free_resource()
{
	reset_args();
}

static int do_exit(int argc, char** argv)
{
	int val = 0;
	if(argc > 1)
		val = atoi(argv[1]);
	free_resource();
	history_finish();
	exit(val);
	return 0;
}

static int do_cd(int argc, char** argv)
{
	char* dir;
	char cwd[100];
	extern char lastdir[];

	if(argc == 1) {
		if(!(dir = getenv("HOME"))) {
			printf("cd: %s\n", strerror(errno));
			return -1;
		}
	} else if(argc == 2) {
		if(strcmp(argv[1], "-") == 0) {
			dir = lastdir;
		} else if(strcmp(argv[1], "~") == 0) {
			if(!(dir = getenv("HOME"))) {
				printf("cd: %s\n", strerror(errno));
				return -1;
			}
		} else
			dir = argv[1];
	} else {
		printf("Usage: cd [dir]\n");
		return -1;
	}
	getcwd(cwd, 99); 
	if(chdir(dir) == -1) {
		printf("cd: %s\n", strerror(errno));
		return -1;
	}
	strcpy(lastdir, cwd);
	return 0;
}

static int do_export(int argc, char** argv)
{
	int i = 1;
	char* p;
	while(argv[i]) {
		if((p = strchr(argv[i], '='))) {
			*p = 0;
			if(strpbrk(argv[i], "~`!@#$%^&*()-_+=|\\{}[];:'\"<>,.?/")) {
				*p = '=';
				printf("export: %s: not a valid indentifier\n", argv[i]);
				i++;
				continue;
			} 

			if(setenv(argv[i], p+1, 1) == -1) 
				printf("export: %s\n", strerror(errno));
			*p = '=';
		}
		i++;		
	}
	return 0;
}

static int do_echo(int argc, char** argv)
{
	int i = 1;
	int j;
	int argn = 0;
	int arge = 0;
	if(argv[1]) {
		if(strcmp(argv[1], "-n") == 0) {
			argn = 1;
			i = 2;
		} else if(strcmp(argv[1], "-e") == 0) {
			arge = 1;
			i = 2;
		} else if((strcmp(argv[1], "-ne") == 0) || (strcmp(argv[1], "-en") == 0)) {
			argn = arge = 1;
			i = 2;
		}
	}
	j = i;
	while(argv[i]) {
		if(i > j)
			printf(" %s", argv[i]);
		else 
			printf("%s", argv[i]);
		i++;
	}
	if(argn == 0)
		printf("\n");
	
	return 0;
}

int do_history(int argc, char** argv)
{
	display_history_list();	
	return 0;
}

static void add_simple_arg(const char* arg)
{
	argbuf[argcnt] = (char*)malloc(strlen(arg)+1);
	strcpy(argbuf[argcnt], arg);
	argcnt++;
	argbuf[argcnt] = 0;
}

// $HOME
// $$
// $HOME$
// $HOME$HOME
static void add_arg(const char* xarg)
{
	char* arg;

	char buf[200];
	char xbuf[200];
	int i,j,k;
	int len = strlen(xarg);
	

	k = 0;

	for(i = 0; i < len; i++) {
		if(xarg[i] == '$') {
			if(xarg[i+1] == '$') {//$$,get pid
				int pid = getpid();
				sprintf(buf+k, "%d", pid);
				k = strlen(buf);
				i++;
			} else if(xarg[i+1] == 0){//$ and end
				buf[k] = '$';
				k++;
				break;
			} else {//$HOME or $HOME$OTHER
				for(j = i+1; j < len; j++) {
					if(xarg[j] == '$') 
						break;
					xbuf[j-i-1] = xarg[j];
				}
				xbuf[j-i-1] = 0;
				i = j-1;
				if((arg = getenv(xbuf))) {
					strcpy(buf+k, arg);
					k += strlen(arg);
				}
			}
			
		} else {
			buf[k] = xarg[i];
			k++; 
		}
	}
	buf[k] = 0;
	if(k > 0)
		add_simple_arg(buf);
}

static int file_exist(const char* file, char* buffer)
{
	int i = 0;
	const char* p;
	const char* path;
	path = getenv("PATH");
	p = path;
	while(*p != 0) {
		if(*p != ':')
			buffer[i++] = *p;
		else {
			buffer[i++] = '/';
			buffer[i] = 0;
			strcat(buffer, file);
			if(access(buffer, F_OK) == 0)
				return 1;
			i = 0;
		}
		p++;
	}
	return 0;
}

static void do_list_cmd()
{
	int i = 0;
	int j = 0;
	char* p;
	while(argbuf[i]) {
		if(strcmp(argbuf[i], ";") == 0) {//	 ;
			p = argbuf[i];
			argbuf[i] = 0;
			do_pipe_cmd(i-j, argbuf+j);
			argbuf[i] = p;
			j = ++i;
		} else
			i++;
	}
	do_pipe_cmd(i-j, argbuf+j);
}

static int do_pipe_cmd(int argc, char** argv)
{
	int i = 0;
	int j = 0;
	int prepipe = 0;
	int prefd[2];
	int postfd[2];
	char* p;

	while(argv[i]) {
		if(strcmp(argv[i], "|") == 0) { // pipe
			p = argv[i];
			argv[i] = 0;

			pipe(postfd); 		//create the post pipe
			//be sure not close pipe in, otherwise whenever father write to this pipe,
			//cause a Broken pipe.
			//close(postfd[0]);	//father close pipe in
			
			if(prepipe)	
				do_simple_cmd(i-j, argv+j, prefd, postfd);
			else
				do_simple_cmd(i-j, argv+j, 0, postfd);
			argv[i] = p;
			prepipe = 1;
			prefd[0] = postfd[0];
			prefd[1] = postfd[1];
			j = ++i;
		} else
			i++;
	}
	if(prepipe)
		do_simple_cmd(i-j, argv+j, prefd, 0);
	else 
		do_simple_cmd(i-j, argv+j, 0, 0);
	return 0;
}


static int predo_for_redirect(int argc, char** argv, int* re)
{
	int i;
	int redirect = 0;	
	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "<") == 0) {
			redirect = 1;
			argv[i] = 0;
			break;
		} else if(strcmp(argv[i], ">") == 0) {
			redirect = 2;
			argv[i] = 0;
			break;	
		}
	}
	if(redirect) {// need redirect stdin or stdout
		if(argv[i+1]) {
			int fd;
			if(redirect == 2) {
				if((fd = open(argv[i+1], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1) {
					fprintf(stderr, "Open out %s failed\n", argv[i+1]);
					return 1;
				}
				dup2(fd, STDOUT_FILENO);
			} else {//redirect == 1
				if((fd = open(argv[i+1], O_RDONLY, S_IRUSR|S_IWUSR)) == -1) {
					fprintf(stderr, "Open in %s failed\n", argv[i+1]);
					return 1;
				}
				dup2(fd, STDIN_FILENO);
			}
		} else {
			fprintf(stderr, "Bad redirect, need more arg\n");
			return 1;
		}
	}
	if(re)
		*re = redirect;
	return 0;
}

static int do_simple_cmd(int argc, char** argv, int prefd[], int postfd[])
{
	int pid;
	int status;
	buildin_cmd_handle hd;
	if(argc == 0)
		return 0;

	if(prefd == 0 && postfd == 0) {
	// a very simple buitin command, not have pre and post pipe, father should do the
	// builtin command itself.
		if((hd = get_cmd_handle(argv[0]))) {
			if(predo_for_redirect(argc, argv, 0))
				return 1;
			(*hd)(argc, argv);
			return 0;
		}
	}
	
	if((pid = fork()) == 0) {//child
		// reset the signal INT handle to default
		int redirect = 0;
		signal(SIGINT, SIG_DFL);

		if(predo_for_redirect(argc, argv, &redirect))
			exit(1);
		
		if(redirect != 1 && prefd) {//has a pre pipe, redirect stdin
			close(prefd[1]);
			if(prefd[0] != STDIN_FILENO) {
	//			fprintf(stderr, "redirect stdin\n");
				dup2(prefd[0], STDIN_FILENO);
				close(prefd[0]);
			}
		}
		if(redirect != 2 && postfd) {//has a post pipe, redirect stdout
			close(postfd[0]);
			if(postfd[1] != STDOUT_FILENO) {
	//			fprintf(stderr, "redirect stdout\n");
				dup2(postfd[1], STDOUT_FILENO);
				close(postfd[1]);
			}
		}
		if((hd = get_cmd_handle(argv[0]))) {
			(*hd)(argc, argv);
			exit(0);
		}

		char buffer[100];
		if(file_exist(argv[0], buffer)) {
	//		fprintf(stderr, "exec command %s\n", buffer);
			execv(buffer, argv);
		}
		else {
			fprintf(stderr, "-msh: %s: command not found\n", argv[0]);
			exit(0);
		}
	}
	waitpid(pid, &status, 0);
	if(postfd) { // no
		close(postfd[1]); // must close this fd here.
	}
	return 0;
}


static void reset_args()
{
	int i;
	for(i = 0; i < argcnt; i++) {
		free(argbuf[i]);
		argbuf[i] = 0;
	}
	argcnt = 0;
}

static int my_yyinput(char* buf, int max)
{
	int n;
	n = (max < (mylim-myptr)) ? max : (mylim-myptr);

	if(n > 0) {
		memcpy(buf, myptr, n);
		myptr += n;
	}
	return n;
}

