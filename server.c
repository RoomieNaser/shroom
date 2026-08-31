#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

void* handle_client(void* arg) {
	int sock = *(int*)arg;
	free(arg);
	char buffer[2048];

	while (1) {
		memset(buffer, 0, sizeof(buffer));
		int readBytes = read(sock, buffer, sizeof(buffer));

		//cleaning up in case of abrupt disconnects
		if (readBytes <= 0) {
			close(sock);
			break;
		}

		//decryption, parse commands, re encrypts

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
