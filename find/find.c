#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

int inodeON = 0;
ino_t inodeNumber;

int fnameON = 0;
char * fileName;

int sizeOn = -2;
off_t fileSize;

int nlinkON = 0;
nlink_t nlinks;

int execOn = 0;
char * execPath;

int foundSomething = 0;

int compareFileSize(off_t mark, off_t fs) {
    if(mark != fs) {
        return mark<fs ? 1 : -1;
    }
    return 0;
}

int filterFile(char * path, char * fn) {
    struct stat sb;
    if (lstat(path, &sb) == -1) {
        char error_str[1024];
        snprintf(error_str, sizeof(error_str), "Can't read file info of %s", path);
        perror(error_str);
        return 0;
        //exit(EXIT_FAILURE);
    }
    if(inodeON && sb.st_ino != inodeNumber) {
        return 0;
    }
    if(fnameON && strcmp(fn, fileName) != 0) {
        return 0;
    }
    if(sizeOn != -2 && compareFileSize(fileSize, sb.st_size) != sizeOn) {
        return 0;
    }
    if(nlinkON && sb.st_nlink != nlinks) {
        return 0;
    }
    foundSomething = 1;
    return 1;
}

void executeCommand(char * * arguments) {
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
                perror("Error waiting for process");
            } else if (wpid == -1) {
                perror("Error while waiting for process");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        //printf("Result code: %i\n",status);
    }
}

void listdir(char * name) {
    DIR *dir;
    struct dirent *entry;
    if (!(dir = opendir(name))) {
        char error_str[1024];
        snprintf(error_str, sizeof(error_str), "Can't open directory %s", name);
        perror(error_str);
        closedir(dir);// :)
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            listdir(path);
        } else {
            if(filterFile(path, entry->d_name)) {
                if(!execOn) {
                    printf("%s\n", path);
                } else {
                    char * arguments[3] = {execPath, path, NULL};
                    executeCommand(arguments);
                    //exit(0);
                }
            }
        }
    }
    closedir(dir);
}

void printUsageAndExit() {
    printf("Usage: find /path/to/folder -inum [inum] -name [name] -size [-+=][size] -nlinks [numOfHardLinks] -exec [pathToExecutableForFileOutput]\n");
    exit(EXIT_FAILURE);
}

int toNum(char * s, long long * answer) {
   long long res = 0;
   long long minus = *s == '-';
   if (minus) s++;
   while(isdigit(*s)) {
      res = res * 10 + (*s++ - '0');
   }
   if (*s) {
       return 0;
   }
   (*answer) = minus ? -res : res;
   return 1;
}

long long getNumber(char * s) {
    long long num;
    if(!toNum(s, &num)) {
        printf("Invalid number: %s\n", s);
        printUsageAndExit();
    }
    return num;
}

void checkArg(int ii, int argc, char ** argv) {
    if(ii + 1 >= argc) {
        printf("Can't find argument's parameter: %s\n", argv[ii]);
        printUsageAndExit();
    }
}

int main(int argc, char ** argv) {
    if(argc <= 1) {
        printUsageAndExit();
    }
    for(int i = 2; i < argc; i+=2) {
        if(strcmp(argv[i], "-inum") == 0) {
            checkArg(i, argc, argv);
            inodeON = 1;
            inodeNumber = getNumber(argv[i+1]);
        } else if(strcmp(argv[i], "-name") == 0) {
            checkArg(i, argc, argv);
            fnameON = 1;
            fileName = argv[i+1];
        } else if(strcmp(argv[i], "-size") == 0) {
            checkArg(i, argc, argv);
            if(strlen(argv[i+1])<2) {
                printf("Invalid size: %s\n", argv[i+1]);
                printUsageAndExit();
            }
            char cc = * argv[i+1];
            if(cc == '+') {
                sizeOn = 1;
            } else if(cc == '-') {
                sizeOn = -1;
            } else if(cc == '=') {
                sizeOn = 0;
            } else {
                printf("Invalid symbol: %c\n", cc);
                printUsageAndExit();
            }
            fileSize = getNumber(argv[i+1]+1);
        } else if(strcmp(argv[i], "-nlinks") == 0) {
            checkArg(i, argc, argv);
            nlinkON = 1;
            nlinks = getNumber(argv[i+1]);
        } else if(strcmp(argv[i], "-exec") == 0) {
            checkArg(i, argc, argv);
            execOn = 1;
            execPath = argv[i+1];
        } else {
            printf("Invalid argument: %s\n", argv[i]);
            printUsageAndExit();
        }
    }
    listdir(argv[1]);
    if(!foundSomething) {
        printf("* NO RESULTS *\n");
    }
    return 0;
}
