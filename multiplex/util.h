#ifndef MULTIPLEX_UTIL
#define MULTIPLEX_UTIL

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>
#include <errno.h>

#define BUFFER_LENGTH 1024
#define DEFAULT_PORT 1337
#define true 1
#define false 0

int sendFully(int socket, const char * buffer, int length) {
    int m = 0, bytesSent = 0;
	while (length-bytesSent > 0 && (m = send(socket, buffer+bytesSent, length-bytesSent, 0)) > 0) {
        bytesSent += m;
    }
    return m;
}

int readFully(int socket, char * buffer, int * length) {
    int n = 0, bytesReceived = 0;
    while ((n = recv(socket, buffer+bytesReceived, BUFFER_LENGTH-bytesReceived, 0)) > 0) {
        bytesReceived += n;
        if(buffer[bytesReceived-1] == '\n') {
            break;
        }
    }
    (*length) = bytesReceived;
    return n;
}
#endif
