#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080

void error(char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[]){
	int serverD, newSocket, valRead;
	struct sockaddr_in address;
	int opt = 1;
	int addrLen = sizeof(address);
	char buffer[2048] = {0};
	char *message = "MOTD: Welcome";

	if((serverD = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		error("socket error");
	}

	if(setsockopt(serverD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		error("setsockopt error");
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if(bind(serverD, (struct sockaddr_in *)&address, sizeof(address))< 0) {
		error("bind error");
	}

	if(listen(serverD, 3) < 0) {
		error("listen error");
	}

	if ((newSocket = accept(serverD, (struct sockaddr_in *)&address, (socklen_t*)&addrLen)) < 0) {
		error("accept error");
	}

	valRead = read(newSocket, buffer, 2048);
	printf("%s\n", buffer);
	send(newSocket, message, strlen(message), 0);
	printf("Client reply sent\n");
	return 0;
}
