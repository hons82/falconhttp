#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

pthread_mutex_t counter_mutex;
int counter = 100000;

struct threadargs {
	char ip[16];
	int port;
};

void *threadexec(void *args) {

	pthread_exit(NULL);
}

int main(int argc, char * argv[]) {
	char request[] = "GET /index.html HTTP1.1";
	char response[8192];
	int fd; /* file descriptor */
	struct sockaddr_in server_address;
	int port = 33333;
	char ip[] = "127.0.0.1";

	if (argc == 2) {
		port = atoi(argv[1]);
	} else if (argc == 3) {
		strncpy(ip, argv[1], 15);
		port = atoi(argv[2]);
	}

	printf("Starting Client... %s:%i\n", ip, port);
	while (counter > 0) {
		counter--;
		if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("client: socket error");
			exit(-1);
		}

		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(port);
		server_address.sin_addr.s_addr = inet_addr(ip);
		memset(&server_address.sin_zero, 0, sizeof(server_address.sin_zero));

		printf("Connecting to Server...(%i remaining)\n", counter);

		if (connect(fd, (struct sockaddr*) &server_address,
				sizeof(server_address)) < 0) {
			perror("connect error");
			exit(-1);
		}

		printf("%s \n", "Connected...");

		send(fd, request, sizeof(request), 0);
		recv(fd, response, sizeof(response), 0);

		printf("Received: %i \n", strlen(response));

		close(fd);
	}
	exit(0);
}
