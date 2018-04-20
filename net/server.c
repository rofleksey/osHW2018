#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>

#include "tftp.h"


void rrq(int sock, struct tftp_request *request){
	struct tftp_packet snd_packet;
	char fullpath[256];
	char *r_path = request->packet.filename;
	char *mode = r_path + strlen(r_path) + 1;
	char *blocksize_str = mode + strlen(mode) + 1;
	int blocksize = atoi(blocksize_str);
	if(blocksize <= 0 || blocksize > DATA_SIZE){
		blocksize = DATA_SIZE;
	}
	if(strlen(r_path) + strlen(root) > sizeof(fullpath) - 1){
		fprintf(stderr, "Requested path is too long: %zu\n", strlen(r_path) + strlen(root));
		return;
	}
	memset(fullpath, 0, sizeof(fullpath));
	strcpy(fullpath, root);
	if(r_path[0] != '/'){
		strcat(fullpath, "/");
	}
	strcat(fullpath, r_path);
	printf("RRQ request: \"%s\", blocksize=%d\n", fullpath, blocksize);

	//if(!strncasecmp(mode, "octet", 5) && !strncasecmp(mode, "netascii", 8)){
	//	// send error packet
	//	return;
	//}

	FILE *fp = fopen(fullpath, "r");
	if(fp == NULL){
		fprintf(stderr, "File not found\n");
		snd_packet.cmd = htons(CMD_ERROR);
		snd_packet.code = htons(1);
		strcpy(snd_packet.data, "file not found");
        send(sock, &snd_packet, sizeof(struct tftp_packet), 0);
		return;
	}
	int s_size = 0;
	ushort block = 1;
	snd_packet.cmd = htons(CMD_DATA);
	do{
		memset(snd_packet.data, 0, sizeof(snd_packet.data));
		snd_packet.block = htons(block);
		s_size = fread(snd_packet.data, 1, blocksize, fp); ///GOVNO
		if(send_packet(sock, &snd_packet, s_size + 4) == -1){
			fprintf(stderr, "Failed to send file to client\n");
			fclose(fp);
            return;
		}
		block++;
	} while(s_size == blocksize);
	printf("File has been successfully sent to client\n");
	fclose(fp);
	return;
}

void run(struct tftp_request * request) {
    int sock;
    struct sockaddr_in server;
	static socklen_t addr_len = sizeof(struct sockaddr_in);
    if(request->size <= 0){
		fprintf(stderr, "Bad request size\n");
        free(request);
        return;
	}
	if((sock = openTimeoutSocket()) < 0){
        free(request);
        return;
	}
    server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = 0;
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
		fprintf(stderr, "Socket bind failed\n");
        goto cleanup;
	}
	if(connect(sock, (struct sockaddr*)&(request->client), addr_len) < 0){
		fprintf(stderr, "Socket connection failed\n");
		goto cleanup;
	}
    switch(request->packet.cmd){
		case CMD_RRQ:
			rrq(sock, request);
			break;
		default:
			fprintf(stderr, "Illegal TFTP operation.\n");
			break;
	}
    cleanup:
	    free(request);
	    close(sock);
}

void help(char ** argv) {
    printf("Usage: %s port\n", argv[0]);
}

int main(int argc, char ** argv) {
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    if (argc < 2) {
        fprintf(stderr, "No port provided\n");
        help(argv);
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("Can't open socket");
        exit(1);
    }
    //bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Server bind failed");
        fprintf(stderr, "Is server already open?\n");
        exit(1);
    }
    printf("Server is running on port %d\n", portno);
    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        struct tftp_request * request;
        request = (struct tftp_request *) malloc(sizeof(struct tftp_request));
		memset(request, 0, sizeof(struct tftp_request));
        request->size = recvfrom(sockfd, &(request->packet), MAX_REQUEST_SIZE, 0,
                        (struct sockaddr *) &(request->client), &client_len);
        printf("Client connected\n");
        request->packet.cmd = ntohs(request->packet.cmd);
        run(request);
    }
    return 0;
}
