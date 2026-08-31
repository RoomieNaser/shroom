#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

void* receive_messages(void* arg){
	int sock = *(int*)arg;
	char buffer[2048];

	while(1) {
	
		memset(buffer, 0, sizeof(buffer));
		int readBytes = read(sock, buffer, sizeof(buffer));

		//same disconnect handling as server
		if (readBytes <= 0){
			printf("Shroom Disconnected :(\n");
			exit(0);
		}

		//Decryption logic goes here
		
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

		//Encrypting with key before sending section here
		
		write(sock, input_buffer, strlen(input_buffer));
	}


	return 0;
}



