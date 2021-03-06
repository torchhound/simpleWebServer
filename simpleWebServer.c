#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#define PORT 8080
#define BUFF_LEN 4096

void error(char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[]){
	int serverD, newSocket, fileSize, notFoundSize;
	FILE *file;
	FILE *notFound;
	struct sockaddr_in address;
	int opt = 1;
	int addrLen = sizeof(address);
	char readBuffer[BUFF_LEN] = {0};
	char *requestSplit;
	char *okResponse = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
	char *notFoundPartial = "HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: ";
	char notFoundCharSize[2048]; //malloc
	char *notFoundResponse;
	char fileCharSize[2048]; //malloc

	notFound = fopen("404.html", "r");
	fseek(notFound, 0, SEEK_END);
	notFoundSize = ftell(notFound);
	rewind(notFound);
	char notFoundBuffer[notFoundSize];
	fread(notFoundBuffer, notFoundSize, 1, notFound);
	sprintf(notFoundCharSize, "%d", notFoundSize);
	int notFoundResponseSize = strlen(notFoundPartial) + strlen(notFoundCharSize) + strlen(notFoundBuffer) + 4; //don't hardcode padding
	notFoundResponse = malloc(notFoundResponseSize);
	strcpy(notFoundResponse, notFoundPartial);
	strcat(notFoundResponse, notFoundCharSize);
	strcat(notFoundResponse, "\r\n\r\n");
	strcat(notFoundResponse, notFoundBuffer);
	printf("Server listening on port %d\n", PORT);

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
	while(1) {
		if(listen(serverD, 3) < 0) {
			error("listen error");
		}
		if ((newSocket = accept(serverD, (struct sockaddr_in *)&address, (socklen_t*)&addrLen)) < 0) {
		  error("accept error");
		}
		memset(readBuffer, '\0', BUFF_LEN);
		read(newSocket, readBuffer, BUFF_LEN);
		requestSplit = strtok(readBuffer, " "); //strtok / on second requestSplit token
		if(strcmp("GET\n", requestSplit) == 0 || strcmp("GET", requestSplit) == 0) {
			file = fopen("index.html", "r");
			if(file == NULL) {
				if(send(newSocket, notFoundResponse, notFoundResponseSize, 0) < notFoundResponseSize) {
					close(newSocket);
					error("404 send error");
				}
				close(newSocket);
				printf("404 sent\n");
			}
			fseek(file, 0, SEEK_END);
			fileSize = ftell(file);
			rewind(file);
			char fileBuffer[fileSize];
			int fileSendSize = fileSize + snprintf(NULL, 0, "%d", fileSize) + strlen(okResponse) + 4;
			char fileSendBuffer[fileSendSize];
			fread(fileBuffer, fileSize, 1, file);
			strcat(fileSendBuffer, okResponse);
			sprintf(fileCharSize, "%d%s", fileSize, "\r\n");
			strcat(fileSendBuffer, fileCharSize);
			strcat(fileSendBuffer, "\r\n");
			strcat(fileSendBuffer, fileBuffer);
			if(send(newSocket, fileSendBuffer, fileSendSize, 0) < fileSendSize) {
				close(newSocket);
				error("file send error");
			}
      printf(fileSendBuffer);
			close(newSocket);
			printf("File served\n");
			fclose(file);
		} else {
			if(send(newSocket, notFoundResponse, notFoundResponseSize, 0) < notFoundResponseSize) {
				close(newSocket);
				error("404 send error");
			}
			close(newSocket);
			printf("404 sent\n");
		}
	}
	fclose(notFound);
	fclose(file);
	free(notFoundResponse);
}