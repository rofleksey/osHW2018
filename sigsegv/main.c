#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <ucontext.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/mman.h>

#define lul unsigned long long
#define bool int

char hex[] = "0123456789ABCDEF";
lul regs[] = {REG_RAX, REG_RBP, REG_RBX, REG_RCX, REG_RDI, REG_RDX, REG_RIP, REG_RSI, REG_RSP};
char * names[] = {"RAX: ", "RBP: ", "RBX: ", "RCX: ", "RDI: ", "RDX: ", "RIP: ", "RSI: ", "RSP: "};
#define REG_LENGTH 9


void printString(char const *buffer) {
	size_t len = strlen(buffer);
	for(size_t i = 0; i < len; ) {
		ssize_t res = write(STDOUT_FILENO, buffer, len);
		if (res == -1) {
			char * err = strerror(errno);
			write(STDERR_FILENO, err, strlen(err));
			exit(EXIT_FAILURE);
		}
		buffer += res;
		i += res;
	}
}

void toHex(lul num, char *hexString) {
	hexString[0] = '0';
    hexString[1] = 'x';
	for (size_t i = 2; i < 18; ++i) {
		hexString[i] = hex[num % 16];
		num /= 16;
	}
	hexString[18] = '\0';
}

void printCharacter(char c) {
	char buf[] = { c };
	printString(buf);
}

void printHex(unsigned char c) {
	printCharacter(hex[c / 16]);
	printCharacter(hex[c % 16]);
}

void handler(int signal, siginfo_t *data, void *extra_data) {
    ucontext_t *p = (ucontext_t *) extra_data;
    printString("=== REGISTERS ===\n");
    char register_dump[50];
    for(int i = 0; i < REG_LENGTH; i++) {
		printString(names[i]);
		toHex(p->uc_mcontext.gregs[regs[i]], register_dump);
		printString(register_dump);
		printString("\n");
	}
	printString("=== END OF REGISTERS ===\n");
    size_t* start_chunk = (size_t*) (data->si_addr) - 32;
    size_t* last_chunk = start_chunk + 64;
    printString("\n=== MEMORY DUMP ===\n");
	bool flag = 0;
	size_t* pointer = start_chunk;
	while (pointer++ != last_chunk) {
		int fd[2];
		if (pipe(fd) >= 0) {
			if (write(fd[1], pointer, 1) > 0) {
			  	unsigned char c = (*(unsigned char*)(pointer)) & 0xFF;
				printHex(c);
			  	printString(" ");
			} else {
				flag = 1;
				printString("** ");
			}
			close(fd[0]);
			close(fd[1]);
		}
	}
    printString("\n=== END OF MEMORY DUMP ===\n");
	if (flag) {
		printString("\n** - couldn't dump this address\n");
	}
	exit(EXIT_FAILURE);
}


int main() {
    struct sigaction sig_act;
    sig_act.sa_flags = SA_SIGINFO;
    sig_act.sa_sigaction = handler;
    sig_act.sa_flags &= SA_RESTART;
    if (sigaction(SIGSEGV, &sig_act, NULL) < 0) {
      	printString("COULDN'T ASSIGN SIGFAULT HANDLER\n");
		exit(EXIT_FAILURE);
    }
    char * mem = NULL;
    mem[0] = ' ';
    return 0;
}
