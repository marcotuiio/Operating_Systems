#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/printk.h>

asmlinkage long sys_hello(void) {
	printk("HELLO WORLD MARCO TULIO DO KERNEL\n");
	return 0;
}