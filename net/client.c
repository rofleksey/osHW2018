#include "tftp.h"

#define LINE_BUF_SIZE 1024

int sock;
socklen_t addr_len;
struct sockaddr_in server;

void do_get(char *remote_file, char *local_file){
    printf("Starting downloading remote file \"%s\"\nto local file \"%s\"\n", remote_file, local_file);
	struct tftp_packet snd_packet, rcv_packet;
	struct sockaddr_in sender;

	int r_size = 0;
	int attempts;
	ushort block = 1;

	snd_packet.cmd = htons(CMD_RRQ);
	sprintf(snd_packet.filename, "%s%c%s%c%d%c", remote_file, 0, "octet", 0, DATA_SIZE, 0);
	sendto(sock, &snd_packet, sizeof(struct tftp_packet), 0, (struct sockaddr*)&server, addr_len);

    FILE *fp = NULL;

	snd_packet.cmd = htons(CMD_ACK);
	do{
		for(attempts = 0; attempts < MAX_CLIENT_ATTEMPTS; attempts++) {
			r_size = recvfrom(sock, &rcv_packet, sizeof(struct tftp_packet), 0,
					(struct sockaddr *)&sender, &addr_len);
            int tmpErrno = errno;
			if(r_size > 0 && r_size < 4) {
				fprintf(stderr, "Invalid packet size: %d\n", r_size);
			}
            if(rcv_packet.cmd == htons(CMD_ERROR)) {
                fprintf(stderr, "Got server error: %s\n", rcv_packet.data);
                return;
            }
			if(r_size >= 4 && rcv_packet.cmd == htons(CMD_DATA) && rcv_packet.block == htons(block)){
				printf("Got data block #%d, block size = %d\n", ntohs(rcv_packet.block), r_size - 4);
				snd_packet.block = rcv_packet.block;
				sendto(sock, &snd_packet, sizeof(struct tftp_packet), 0, (struct sockaddr*)&sender, addr_len);
                if(fp == NULL) {
                    fp = fopen(local_file, "w");
                    if(fp == NULL) {
                        fprintf(stderr, "Can't create file %s\n", local_file);
                        return;
                    }
                }
				fwrite(rcv_packet.data, 1, r_size - 4, fp);
				break;
			} else {
                if(r_size < 0 && (tmpErrno == EAGAIN || tmpErrno == EWOULDBLOCK)) {
					fprintf(stderr, "Data block #%d timed out, retrying...\n", block);
				} else {
					fprintf(stderr, "Data block #%d is invalid, retrying...\n", block);
				}
            }
		}
		if(attempts >= MAX_CLIENT_ATTEMPTS) {
			fprintf(stderr, "Coudn't acquire data block #%d after %d retreivals.\n", block, MAX_CLIENT_ATTEMPTS);
            fprintf(stderr, "File download failed.\n");
            if(fp != NULL) {
    		    fclose(fp);
            }
            return;
		}
		block++;
	} while(r_size == DATA_SIZE + 4);
    printf("File has been successfully downloaded\n");
    if(fp != NULL) {
	    fclose(fp);
    }
}

void help(char ** argv) {
    printf("Usage: %s server_ip [server_port]\nIf no port is specified, port %d will be used.\n", argv[0], SERVER_PORT);
}

int main(int argc, char **argv){
	char cmd_line[LINE_BUF_SIZE];
	char *buf;
	char *arg;
	char *local_file;
	char *server_ip;
	unsigned short port = SERVER_PORT;
	addr_len = sizeof(struct sockaddr_in);
	if(argc < 2){
        help(argv);
		return 1;
	}

	server_ip = argv[1];
	if(argc > 2){
		port = (unsigned short)atoi(argv[2]);
	}

	if((sock = openTimeoutSocket()) < 0){
		fprintf(stderr, "Couldn't create server socket\n");
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
    if(inet_pton(AF_INET, server_ip, &(server.sin_addr.s_addr)) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", server_ip);
        help(argv);
        return 1;
    }

    printf("Using server %s:%d\n", server_ip, port);

	while(1){
		printf("# ");
		memset(cmd_line, 0, LINE_BUF_SIZE);
		buf = fgets(cmd_line, LINE_BUF_SIZE, stdin);

		if(buf == NULL){
			printf("See ya\n");
			return 0;
		}

		arg = strtok (buf, " \t\n");
		if(arg == NULL){
			continue;
		}

		if(strcmp(arg, "get") == 0){
			arg = strtok (NULL, " \t\n");
			local_file = strtok (NULL, " \t\n");
			if(arg == NULL){
				fprintf(stderr, "Missing arguments. Usage: get remote_file [local_file]\nIf local_file is not specified, it's name will be equal to remote_file\n");
			} else {
				if(local_file == NULL){
					local_file = arg;
				}
				do_get(arg, local_file);
			}
		} else if(strcmp(arg, "quit") == 0){
			break;
		} else{
			printf("Invalid command.\nAvailable commands: get, quit\n");
		}

	}
	return 0;
}
