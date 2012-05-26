#include <readline/readline.h>
#include <stdio.h>

int main()
{
	char *p;
	p = readline("请输入：");
	printf("%s\n",p);
	free(p);
	return 0;
}
	
