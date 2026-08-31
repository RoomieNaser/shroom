#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "file_utils.h"
#include "cipher.h"

char my_key[50] = "";

void* receive_messages(void* arg){
	int sock = *(int*)arg;
	char buffer[MAX_FILE_SIZE + 1024];

	while(1) {
	
		memset(buffer, 0, sizeof(buffer));
		int readBytes = read(sock, buffer, sizeof(buffer));

		//same disconnect handling as server
		if (readBytes <= 0){
			printf("Shroom Disconnected :(\n");
			exit(0);
		}

		if (strlen(my_key) > 0) {
            		xor_cipher(buffer, readBytes, my_key);
        	}

		if (strncmp(buffer, "RECVFILE FROM ", 14) == 0) {
    			char sender[50], filename[256];
    			long file_size;
    			char *newline_pos = strchr(buffer, '\n');
    			if (newline_pos) {
        			*newline_pos = '\0';
        			sscanf(buffer, "RECVFILE FROM %s %s %ld", sender, filename, &file_size);
        			*newline_pos = '\n';
        			char *payload = newline_pos + 1;
        			save_received_file(filename, payload, file_size);
        			printf("\nRECVFILE FROM %s: %s (%ld bytes)\n[content saved to ./received_%s]\n> ", sender, filename, file_size, filename);
        			fflush(stdout);
        			continue;
    			}
		}
		printf("\n%s\n> ", buffer);
		fflush(stdout);
	}
	return NULL;
}

//main stuff for client
int main(int argc, char const *argv[]){
	if (argc != 3){
		printf("Usage: ./client <server_ip> <port>\n");
		return 1;
	}

	int sock = 0;
	struct sockaddr_in serv_addr;
	char input_buffer[2048];

	sock = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	
	inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
	connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	printf("Welcome to Shroom. Ready to Register client!\n");

	//thread spawning to handle async server messages
	pthread_t recv_thread;
	pthread_create(&recv_thread, NULL, receive_messages, (void*)&sock);

	while(1) {
	
		printf("> ");
		fgets(input_buffer, sizeof(input_buffer), stdin);

		input_buffer[strcspn(input_buffer, "\n")] = 0;
		
		//if input is QUIT the server cleans up without affecting other clients
		if (strcmp(input_buffer, "QUIT") == 0){
			write(sock, input_buffer, strlen(input_buffer));
			close(sock);
			break;
		
		}

		// Save the key when registering
		if (strncmp(input_buffer, "REGISTER ", 9) == 0) {
    			sscanf(input_buffer, "REGISTER %*s KEY %s", my_key);
		}
		// Encrypt standard messages
		if (strlen(my_key) > 0 && strncmp(input_buffer, "SENDFILE", 8) != 0) {
    			xor_cipher(input_buffer, strlen(input_buffer), my_key);
		}
		if (strncmp(input_buffer, "SENDFILE TO ", 12) == 0) {
    			char target[50], filepath[256];
    			if (sscanf(input_buffer, "SENDFILE TO %[^:]: %s", target, filepath) == 2) {
        			if (!is_txt_extension(filepath)) {
            				printf("ERROR only .txt files are supported\n");
            				continue;
        			}
        			char file_payload[MAX_FILE_SIZE];
        			long file_size = read_file_content(filepath, file_payload);
        			if (file_size < 0) {
            				printf(file_size == -1 ? "ERROR file not found\n" : "ERROR file exceeds limit\n");
            				continue;
        			}
        			char *filename = strrchr(filepath, '/');
        			filename = (filename) ? filename + 1 : filepath;
        			char network_frame[MAX_FILE_SIZE + 1024];
        			int header_len = sprintf(network_frame, "SENDFILE TO %s %s %ld\n", target, filename, file_size);
        			memcpy(network_frame + header_len, file_payload, file_size);
				xor_cipher(network_frame, header_len + file_size, my_key);
        			write(sock, network_frame, header_len + file_size);
        			continue;
    			}
		}
		write(sock, input_buffer, strlen(input_buffer));
	}


	return 0;
}



