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
#define CONNECTIONS_NUMBER 5
#define true 1
#define false 0

void printUsageAndExit() {
	fprintf(stderr, "Usage: ./client address port\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	signal(SIGPIPE, SIG_IGN);
	if(argc != 3) {
		printUsageAndExit();
	}
	char * address = argv[1];
	int port = atoi(argv[2]);
	if(port <= 0) {
		fprintf(stderr, "Invalid port number\n");
		printUsageAndExit();
	}
	struct sockaddr_in serverAddressIn;
	serverAddressIn.sin_family = AF_INET;
	if(inet_pton(AF_INET, address, &(serverAddressIn.sin_addr.s_addr)) <= 0) {
		perror("Invalid address");
		exit(EXIT_FAILURE);
	}
	serverAddressIn.sin_port = htons(port);
	int clients[CONNECTIONS_NUMBER];
	for(int i = 0; i < CONNECTIONS_NUMBER; i++) {
		int sock;
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("Could not create socket");
			exit(EXIT_FAILURE);
		}
		if (connect(sock, (struct sockaddr*)&serverAddressIn,
		            sizeof(serverAddressIn)) < 0) {
			perror("Could not connect to server");
			exit(EXIT_FAILURE);
		}
		struct timeval timeout;
		timeout.tv_sec = 60;
		timeout.tv_usec = 0;
		if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
			perror("Couldn't set socket option");
			exit(EXIT_FAILURE);
		}
		if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
			perror("Couldn't set socket option");
			exit(EXIT_FAILURE);
		}
		printf("Client (fd %d) has connected to server\n", sock);
		clients[i] = sock;
	}
	int activity, curSock, readResult, maxSocketDescriptor;
	fd_set clientsSet;
	char buffer[BUFFER_LENGTH];
	int clientsBurriedAliveRIP2018 = 0;
	while(clientsBurriedAliveRIP2018 != CONNECTIONS_NUMBER) {
		FD_ZERO(&clientsSet);
		maxSocketDescriptor = clients[0];
		for (int i = 0; i < CONNECTIONS_NUMBER; i++) {
			curSock = clients[i];
			if(curSock > 0) {
				FD_SET(curSock, &clientsSet);
			}
			if(curSock > maxSocketDescriptor) {
				maxSocketDescriptor = curSock;
			}
		}
		activity = select(maxSocketDescriptor + 1, &clientsSet, NULL, NULL, NULL);
		if ((activity < 0) && (errno != EINTR)) {
			perror("Multiplexing error");
		}
		for (int i = 0; i < CONNECTIONS_NUMBER; i++) {
			curSock = clients[i];
			if (FD_ISSET(curSock, &clientsSet)) {
				if ((readResult = read(curSock, buffer, BUFFER_LENGTH)) <= 0) {
					printf("Client (fd %d) has been violently disintegrated and it's unsatisfied soul is still wandering around the globe to find peace and soup\n", curSock);
				} else{
					buffer[readResult] = '\0';
					printf("Client (fd %d) is replying to server...\n", curSock);
					send(curSock, buffer, strlen(buffer), 0);
					printf("Client (fd %d) has replied and disconnected\n", curSock);
				}
				close(curSock);
				clients[i] = 0;
				clientsBurriedAliveRIP2018++;
			}
		}
	}
	printf("I'm done\n");
	return 0;
}
