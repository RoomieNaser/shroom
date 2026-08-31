#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "cipher.h"
#include "file_utils.h"

#define MAX_CLIENTS 100

//for registration stuff
typedef struct {
	char username[50];
	char key[50];
	int socket;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;

//using pthread's built in mutex instead of building one
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void* handle_client(void* arg){
	char current_user[50] = "";
	int sock = *(int*)arg;
	free(arg);
	char buffer[MAX_FILE_SIZE + 1024];

	while (1) {
		memset(buffer, 0, sizeof(buffer));
		int readBytes = read(sock, buffer, sizeof(buffer));

		//cleaning up in case of abrupt disconnects
		if (readBytes <= 0 || strncmp(buffer, "QUIT", 4) == 0) {
			pthread_mutex_lock(&clients_mutex);
			for (int i = 0; i < client_count; i++){
				if (clients[i].socket == sock){
					for (int j = i; j < client_count - 1; j++){
						//leftShift the remaining clients to remove disconnected user
						clients[j] = clients[j + 1];
					}
					client_count--;
					break;
				}
			}
			pthread_mutex_unlock(&clients_mutex);

			if (strncmp(buffer, "QUIT", 4) == 0){
				char goodbye[150];
				sprintf(goodbye, "GOODBYE %s, Shroom will be lonely without you D: \n", current_user);
				write(sock, goodbye, strlen(goodbye));
			}

			close(sock);
			break;
		}

		char sender_key[50] = "";
		pthread_mutex_lock(&clients_mutex);
		for (int i = 0; i < client_count; i++) {
    			if (clients[i].socket == sock) strcpy(sender_key, clients[i].key);
		}
		pthread_mutex_unlock(&clients_mutex);

		if (strlen(sender_key) > 0) {
    			xor_cipher(buffer, readBytes, sender_key);
		}

		char cmd[20], username[50], key_label[20], key[50];

		if (sscanf(buffer, "%s %s %s %s", cmd, username, key_label, key) == 4){
			if (strcmp(cmd, "REGISTER") == 0 && strcmp(key_label, "KEY") == 0){
				pthread_mutex_lock(&clients_mutex);
				int username_taken = 0;

				for (int i = 0; i < client_count; i++){
					if (strcmp(clients[i].username, username) == 0){
						username_taken = 1;
						break;
					}
				}

				if (username_taken){
					char error_msg[100];
					sprintf(error_msg, "WOMP WOMP username %s is already taken gng\n", username);
					write(sock, error_msg, strlen(error_msg));
				} else if (client_count < MAX_CLIENTS){
					strcpy(clients[client_count].username, username);
					strcpy(clients[client_count].key, key);
					clients[client_count].socket = sock;
					client_count++;

					char success_msg[100];
					sprintf(success_msg, "Registration Successful %s, YIPPEE\n", username);
					write(sock, success_msg, strlen(success_msg));
					printf("Welcome to Shroom, %s!\n", username);
					strcpy(current_user, username);
				}
				pthread_mutex_unlock(&clients_mutex);
				continue;
			}
		}

		//sendTo routing setup
		char target[50];
		char msg[1024];

		if (sscanf(buffer, "SEND TO %49[^:]:%1023[^\n]", target, msg) == 2){
			char* actual_msg = msg;
			//skipping whitespaces in the beginning
			if (actual_msg[0] == ' ') actual_msg++;

			pthread_mutex_lock(&clients_mutex);
			int found = 0;

			for (int i = 0; i < client_count; i++){
				if (strcmp(clients[i].username, target) == 0){
					char formatted_msg[1200];
					sprintf(formatted_msg, "FROM %s: %s\n", current_user, actual_msg);
					xor_cipher(formatted_msg, strlen(formatted_msg), clients[i].key);
					write(clients[i].socket, formatted_msg, strlen(formatted_msg));
					found = 1;
					break;
				}
			}

			if (!found){
				char error_msg[100];
				sprintf(error_msg, "ERROR: %s is not online\n", target);
				write(sock, error_msg, strlen(error_msg));
			}

			pthread_mutex_unlock(&clients_mutex);
			continue;
		}
		
		//File routing
		if (strncmp(buffer, "SENDFILE TO ", 12) == 0) {
    			char target[50], filename[256];
    			long file_size;

    			// Locate the newline separating the header from the file payload
    			char *newline_pos = strchr(buffer, '\n');
    			if (newline_pos) {
        		*newline_pos = '\0';
        		sscanf(buffer, "SENDFILE TO %s %s %ld", target, filename, &file_size);
        		*newline_pos = '\n';

        		int header_len = (newline_pos - buffer) + 1;
        		char *payload = buffer + header_len;

        		pthread_mutex_lock(&clients_mutex);
        		int found = 0;
        		for (int i = 0; i < client_count; i++) {
            			if (strcmp(clients[i].username, target) == 0) {
                			char network_frame[MAX_FILE_SIZE + 1024];
                			// Mirror the hop-by-hop routing by re-packaging as RECVFILE
                			int out_header = sprintf(network_frame, "RECVFILE FROM %s %s %ld\n", current_user, filename, file_size);
                			memcpy(network_frame + out_header, payload, file_size);
					xor_cipher(network_frame, out_header + file_size, clients[i].key);

                			write(clients[i].socket, network_frame, out_header + file_size);
                			found = 1;
                			break;
            			}
        		}

        		if (!found) {
            			char error_msg[100];
            			sprintf(error_msg, "ERROR %s is not online\n", target);
            			write(sock, error_msg, strlen(error_msg));
        		}
        		pthread_mutex_unlock(&clients_mutex);
        		continue;
    			}
		}
		//garbage fallback
		if (strncmp(buffer, "SEND ", 5) == 0 || strncmp(buffer, "SENDFILE ", 9) == 0 || strncmp(buffer, "REGISTER", 9) == 0){
			char err_format[] = "ERROR: Invalid command format\n";
			write(sock, err_format, strlen(err_format));
		} else {
			char err_unknown[] = "ERROR unknown command\n";
			write(sock, err_unknown, strlen(err_unknown));
		}

		//decryption, re encrypts

		printf("Received bytes: %d\n", readBytes);
	}
	pthread_exit(NULL);
}	

//main stuff
int main(int argc, char const *argv[]){
	if (argc != 2){
		printf("Usage: ./server <port>\n");
		return 1;
	}

	int serverfd;
	struct sockaddr_in address;
	int opt = 1;
	int port = atoi(argv[1]);
	serverfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	bind(serverfd, (struct sockaddr *)&address, sizeof(address));
	listen(serverfd, SOMAXCONN);
	printf("Shroom listening on port %d\n", port);

	while(1) {
		struct sockaddr_in client_addr;
		socklen_t addrlen = sizeof(client_addr);

		int* client_socket = malloc(sizeof(int));
		*client_socket = accept(serverfd, (struct sockaddr *)&client_addr, &addrlen);

		//spawing the actual threads
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, handle_client, (void*)client_socket);
		pthread_detach(thread_id);
	}


	return 0;
}
