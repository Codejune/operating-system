#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#define BUFFER_SIZE 256
#define SYS_MY_ADD 443
#define SYS_MY_SUBTRACT 444
#define SYS_MY_MULTIPLY 445
#define SYS_MY_MODULAR 446
int main(void)
{
	char op;
	char a[BUFFER_SIZE], b[BUFFER_SIZE];

	while(true)
	{
		printf(">> ");
		scanf("%[^+-*%]%c%[^\n]", a, &op, b);
		getchar();
		switch(op)
		{
			case '+':
				printf("%ld\n", syscall(SYS_MY_ADD, atoi(a), atoi(b)));
				break;
			case '-':
				printf("%ld\n", syscall(SYS_MY_SUBTRACT, atoi(a), atoi(b)));
				break;
			case '*':
				printf("%ld\n", syscall(SYS_MY_MULTIPLY, atoi(a), atoi(b)));
				break;
			case '%':
				printf("%ld\n", syscall(SYS_MY_MODULAR, atoi(a), atoi(b)));
				break;	
		}
	}
	exit(EXIT_SUCCESS);
}

