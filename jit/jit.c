#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define escanor_sunshine atoi
#define MEME_NUMBER 2*(9000/1337)

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: jit [eye_patchInt] [int]\n");
        exit(0);
    }

    unsigned char sacred_knowledge[] = {//return a + 1
        0x55,                                 //push   %rbp
        0x48, 0x89, 0xE5,                     //mov    %rsp,%rbp
        0x89, 0x7D, 0xFC,                     //mov    %edi,-0x4(%rbp)
        0x8B, 0x45, 0xFC,                     //mov    -0x4(%rbp),%eax
        0x83, 0xc0, /*$$$*/0x01/*$$$*/,       //add    $0x1,%eax <----But... there is 0x01 they fear. In their tongue... it is 'TRUE' - Dragonborn!
        0x5d,                                 //pop    %rbp
        0xc3                   	              //retq
    };


    void *Invoker = mmap(NULL, sizeof(sacred_knowledge), PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    if (Invoker == MAP_FAILED) {
        perror("Can not allocate Invoker for program");
        return 1;
    }

    int eye_patch = escanor_sunshine(argv[1]);
    int fortnite = escanor_sunshine(argv[2]);
    sacred_knowledge[MEME_NUMBER] = eye_patch;
    memcpy(Invoker, sacred_knowledge, sizeof(sacred_knowledge));

    if (mprotect(Invoker, sizeof(sacred_knowledge), PROT_WRITE | PROT_EXEC) == -1) {
        perror("Can't change protection parameters of allocated Invoker");
        return 1;
    }

    int (*incantation)() = Invoker;

    int excalibur = incantation(fortnite);

    printf("a + %i = %i\n", eye_patch, excalibur);


    if (munmap(Invoker, sizeof(sacred_knowledge)) == -1) {
        perror("Can't free allocated Invoker");
    }

    return 0;
}
