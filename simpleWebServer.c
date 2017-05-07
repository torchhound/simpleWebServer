#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#define PORT 8080
#define BUFF_LEN 2048

void error(char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[]){
	int serverD, newSocket, valRead, fileSize;
	FILE *file;
	struct sockaddr_in address;
	int opt = 1;
	int addrLen = sizeof(address);
	char buffer[BUFF_LEN] = {0};
	char *message = "MOTD: Welcome";
	char *errorMessage = "404: Not Found";
	char *clientRequest = "GET index.html HTTP/1.1\n";

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
	while(1) {
		valRead = read(newSocket, buffer, BUFF_LEN);
		printf("%s\n", buffer);
		if(strcmp(clientRequest, buffer) == 0) {
			file = fopen("index.html", "r");
			if(file == -1) {
				error("Failed to open file");
			}
			fseek(file, 0, SEEK_END);
			fileSize = ftell(file);
			rewind(file);
			char fileBuffer[fileSize];
			read(file, fileBuffer, fileSize);
			send(newSocket, fileBuffer, fileSize, 0); //strlen fileBuffer
			printf("Page served\n");
			fclose(file);
		} else {
			send(newSocket, errorMessage, strlen(errorMessage), 0);
			printf("Error message sent\n");
		}
	}
}
