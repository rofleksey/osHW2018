#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <signal.h>
#include <sys/ucontext.h>
#include <stdlib.h>
#include <execinfo.h>
#include <ucontext.h>
#include <stdio.h>

void handler(int signal, siginfo_t *data, void *extra_data) {
    ucontext_t *p = (ucontext_t *) extra_data;
    printf("====== SIGSEGV ======\n\n");
    printf("=== REGISTERS DUMP ===\n");
    printf("RSI = %d\n", (int) p->uc_mcontext.gregs[REG_RSI]);
    printf("RSP = %d\n", (int) p->uc_mcontext.gregs[REG_RSP]);
    printf("RAX = %d\n", (int) p->uc_mcontext.gregs[REG_RAX]);
    printf("RCX = %d\n", (int) p->uc_mcontext.gregs[REG_RCX]);
    printf("RIP = %d\n", (int) p->uc_mcontext.gregs[REG_RIP]);
    printf("RDI = %d\n", (int) p->uc_mcontext.gregs[REG_RDI]);
    printf("RBP = %d\n", (int) p->uc_mcontext.gregs[REG_RBP]);
    printf("RDX = %d\n", (int) p->uc_mcontext.gregs[REG_RDX]);
    printf("RBX = %d\n", (int) p->uc_mcontext.gregs[REG_RBX]);
    printf("\n=== MEMORY DUMP ===\n");
    void *dump[100];
    char **line;
    int size = backtrace(dump, 50);
    dump[0] = (void *) p->uc_mcontext.fpregs->rip;
    if((line = backtrace_symbols(dump, size)) == NULL) {
        exit(1);
    }
    for (int i = 1; i < size; ++i) {
        printf("%s\n", line[i]);
    }
    exit(1);
}


int main() {
    struct sigaction sig_act;
    sig_act.sa_flags = SA_SIGINFO;
    sig_act.sa_sigaction = handler;
    sigaction(SIGSEGV, &sig_act, NULL);
    int * lul = NULL;
    printf("%d", *lul);
    return 0;
}
