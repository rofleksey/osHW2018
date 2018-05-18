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
#define MAX_CLIENTS 10
#define true 1
#define false 0

void printUsageAndExit() {
	fprintf(stderr, "Usage: ./server [port]\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
	//ГРИБНОЙ СУП БЫЛ ОЧЕНЬ ВКУСНЫЙ, НО ОН ВНЕЗАПНО КОНЧИЛСЯ, И МНЕ СТАЛО ГРУСТНО :(
	signal(SIGPIPE, SIG_IGN);
	int opt = true;
	int masterSock, addrlen, clientSock, clients[MAX_CLIENTS], activity, readResult, curSock;
	int maxSocketDescriptor;
	struct sockaddr_in address;
	char buffer[BUFFER_LENGTH];
	fd_set clientsSet;
	char * greetingsTraveler = "Yanny or Laurel?";
	int port = DEFAULT_PORT;
	if(argc > 2) {
		printUsageAndExit();
	} else if(argc == 2) {
		if(strcmp("/?", argv[1]) == 0 || strcmp("-h", argv[1]) == 0 || strcmp("--help", argv[1]) == 0) {
			printUsageAndExit();
		} else {
			port = atoi(argv[1]);
			if(port <= 0) {
				fprintf(stderr, "Invalid port\n");
				printUsageAndExit();
			}
		}
	}
	for (int i = 0; i < MAX_CLIENTS; i++) {
		clients[i] = 0;
	}
	if((masterSock = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
		perror("Can't create socket");
		exit(EXIT_FAILURE);
	}
	if(setsockopt(masterSock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
		perror("Can't change socket options");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(masterSock, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("Can't bind socket to default address");
		exit(EXIT_FAILURE);
	}
	printf("Listening to incoming connections on port %d\n", port);
	if (listen(masterSock, MAX_CLIENTS) < 0) {
		perror("Can't open port for listening");
		exit(EXIT_FAILURE);
	}
	addrlen = sizeof(address);
	while(true) {
		FD_ZERO(&clientsSet);
		FD_SET(masterSock, &clientsSet);
		//needed for select
		maxSocketDescriptor = masterSock;
		for (int i = 0; i < MAX_CLIENTS; i++) {
			curSock = clients[i];
			//if is valid (i.e. connected) client
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
		//INCOMING CONNECTION
		if (FD_ISSET(masterSock, &clientsSet)) {
			if ((clientSock = accept(masterSock, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
				perror("Can't accept client connection");
				exit(EXIT_FAILURE);
			}
			printf("Client (fd %d) connected [%s:%d]\n", clientSock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
			if(send(clientSock, greetingsTraveler, strlen(greetingsTraveler), 0) != strlen(greetingsTraveler)) {
				perror("Failed to send data to client");
			}
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if(clients[i] == 0) {
					clients[i] = clientSock;
					break;
				}
			}
		}
		//IO
		for (int i = 0; i < MAX_CLIENTS; i++) {
			curSock = clients[i];
			if (FD_ISSET(curSock, &clientsSet)) {
				if ((readResult = read( curSock, buffer, BUFFER_LENGTH)) <= 0) {
					getpeername(curSock, (struct sockaddr*)&address, (socklen_t*)&addrlen);
					close(curSock);
					clients[i] = 0;
					if(readResult == 0) {
						printf("Client (fd %d) [%s:%d] has been disconnected\n", curSock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
					} else {
						printf("Client (fd %d) [%s:%d] has been violently disintegrated and it's unsatisfied soul is still wondering around the globe to find peace and soup\n", curSock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
					}
				} else{
					buffer[readResult] = '\0';
					printf("Client (fd %d) [%s:%d] replied: '%s'\n", curSock, inet_ntoa(address.sin_addr), ntohs(address.sin_port), buffer);
				}
			}
		}
	}
	//I'm done
	return 0;
}
