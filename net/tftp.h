#ifndef TFTP
#define TFTP

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


#define CMD_RRQ (short)1
#define CMD_WRQ (short)2
#define CMD_DATA (short)3
#define CMD_ACK (short)4
#define CMD_ERROR (short)5
#define CMD_LIST (short)6
#define CMD_HEAD (short)7

#define MAX_REQUEST_SIZE 1024
#define MAX_RESEND_ATTEMPTS 5
#define MAX_RETREIVE_ATTEMPTS 3
#define MAX_CLIENT_ATTEMPTS 10
#define DATA_SIZE 512

#define SERVER_PORT 2018

char * root = "./tftp_data";


struct tftp_packet{
	ushort cmd;
	union{
		ushort code;
		ushort block;
		char filename[2];
	};
	char data[DATA_SIZE];
};

struct tftp_request{
	int size;
	struct sockaddr_in client;
	struct tftp_packet packet;
};

int openTimeoutSocket() {
	int sock;
	if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		printf("Couldn't create socket\n");
		return sock;
	}
	struct timeval timeout = {1,0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(struct timeval));
	return sock;
}

int send_packet(int sock, struct tftp_packet *packet, int size){
	struct tftp_packet rcv_packet;
	int retreive = 0;
	int resend = 0;
	int r_size = 0;

	int timeout = 0;
	for(resend = 0; resend < MAX_RESEND_ATTEMPTS; resend ++){
		if(resend == 0) {
			printf("Sending block=%d\n", ntohs(packet->block));
		} else {
			if(timeout == 0) {
				printf("Invalid client reply. Resending block=%d (attempt #%d)\n", ntohs(packet->block), resend+1);
			} else {
				printf("Client reply timeout. Resending block=%d (attempt #%d)\n", ntohs(packet->block), resend+1);
			}
		}
		timeout = 0;
		if(send(sock, packet, size, 0) != size){
			return -1;
		}
		for(retreive = 0; retreive < MAX_RETREIVE_ATTEMPTS; retreive ++){
			r_size = recv(sock, &rcv_packet, sizeof(struct tftp_packet), 0);
			int tmpErrno = errno;
			if(r_size >= 4 && rcv_packet.cmd == htons(CMD_ACK) && rcv_packet.block == packet->block){
				break;
			} else {
				if(r_size < 0 && (tmpErrno == EAGAIN || tmpErrno == EWOULDBLOCK)) {
					timeout = 1;
				} else {
					timeout = 0;
				}
			}
		}
		if(retreive < MAX_RETREIVE_ATTEMPTS){
			break;
		} else {
			continue;
		}
	}
	if(resend == MAX_RESEND_ATTEMPTS){
		if(timeout) {
			fprintf(stderr, "Client timed out after resending packet for %d times.\n", MAX_RESEND_ATTEMPTS);
		} else {
			fprintf(stderr, "Got no valid feedback from client after resending packet for %d times.\n", MAX_RESEND_ATTEMPTS);
		}
		return -1;
	}

	return size;
}

#endif

/*
Error Codes
   Value     Meaning
   0         Not defined, see error message (if any).
   1         File not found.
   2         Access violation.
   3         Disk full or allocation exceeded.
   4         Illegal TFTP operation.
   5         Unknown transfer ID.
   6         File already exists.
   7         No such user.
*/
