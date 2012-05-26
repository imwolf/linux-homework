#include <stdio.h>

long unsigned int fib(unsigned int n)
{
	int f1=0,f2=1;
	int f=0;
	int i;

	
	for (i=0;i<n;i++){
		if ( i > 1){
			f =  f1 + f2;
			f1 = f2;
			f2 = f;
		}else{
			f = i; //0 or 1
		}
	}
	return f;
}

void main(){
	int n;
	printf("Input Number Of Items:\n");
	scanf("%d",&n);
	fib(n);
	printf("%ld\n",fib(n));
}
