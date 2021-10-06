#include <linux/kernel.h>
#include <linux/syscalls.h>
asmlinkage long sys_my_subtract(long a, long b)
{
	printk("sys_my_subtract: a=%ld, b=%ld, result=%ld\n", a, b, a-b);
	return a - b;
}
SYSCALL_DEFINE2(my_subtract, long, a, long, b)
{
	return sys_my_subtract(a, b);
}

