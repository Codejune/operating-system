#include <linux/kernel.h>
#include <linux/syscalls.h>
asmlinkage long sys_my_multiply(long a, long b)
{
	printk("sys_my_multiply: a=%ld, b=%ld, result=%ld\n", a, b, a*b);
	return a * b;
}

SYSCALL_DEFINE2(my_multiply, long, a, long, b)
{
	return sys_my_multiply(a, b);
}
