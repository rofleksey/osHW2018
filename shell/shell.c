#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

char * getLine() {
    char * arguments_line = NULL;
    size_t length = 0;
    int number = getline(&arguments_line, &length, stdin);
    if (number == -1 && errno == 0) {
        printf("\n");
        exit(0);
    }
    return arguments_line;
}

char ** extractArgs(char *arguments_line) {
    int bufsize = 64;
    int position = 0;
    char ** arguments = malloc(bufsize * sizeof(char *));
    char * argument = NULL;
    if (!arguments) {
        fprintf(stderr, "Memory allocation error!\n");
        exit(1);
    }
    if (arguments_line == NULL) {
        arguments[0] = "exit";
        return arguments;
    }
    argument = strtok(arguments_line, " \t\r\n\a");
    while (argument != NULL) {
        arguments[position] = argument;
        position++;
        if (position >= bufsize) {
            bufsize += 64;
            argument = realloc(arguments, bufsize * sizeof(char*));
            if (!arguments) {
                fprintf(stderr, "Memory allocation error!\n");
                exit(1);
                return 0;
            }
        }
        argument = strtok(NULL, " \t\r\n\a");
    }
    arguments[position] = NULL;
    return arguments;
}

int executeCommand(char ** arguments) {
    if (!strcmp(arguments[0],"exit")) {
        return 0;
    }
    pid_t pid = fork();
    int status;
    if (pid == 0) {
        char * envp[] = {NULL};
        if (execve(arguments[0], &arguments[0],envp) == -1) {
            perror("Execution error");
        }
        exit(0);
    } else if (pid < 0) {
        perror("Fork error");
    } else {
        do {
            pid_t wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == 0) {
                perror("Error");
            } else if (wpid == -1) {
                perror("Error while waiting for process");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        printf("%i\n",status);
    }
    return 1;
}


int main(int argc, char const * argv[]) {
    char * arguments_line = NULL;
    char ** arguments = NULL;
    int next = 1;
    if (argc > 1) {
        printf("No arguments expected\n");
        return 0;
    }
    while(next) {
        printf("> ");
        arguments_line = getLine();
        arguments = extractArgs(arguments_line);
        next = executeCommand(arguments);
        free(arguments_line);
        free(arguments);
    }
    return 0;
}
