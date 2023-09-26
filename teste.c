#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main() {
	long int amma = syscall({numero_da_syscall}); // numero_da_syscall é o numero adicionado em syscall_64.tbl
	printf("System call sys_hello customizada retornou: %ld\n", amma);
	
	return 0;
}