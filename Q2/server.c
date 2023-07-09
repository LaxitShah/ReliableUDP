#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8090
#define SIZE 2000

void saveFile(int sockfd)
{
	int recvCount;
	FILE *fp;
	char *filename = "sampleDataFromCLient.txt";
	char buffer[SIZE];

	fp = fopen(filename, "w");
	while (1)
	{
		recvCount = recv(sockfd, buffer, SIZE, 0);
		if (recvCount <= 0)
		{
			break;
		}
		printf("\n%s", buffer);
		fprintf(fp, "%s", buffer);
		memset(buffer, 0, SIZE);
	}
	return;
}

int main()
{
	int sockFD;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[SIZE];
	char *hello = "Hello from server";

	sockFD = socket(AF_INET, SOCK_STREAM, 0);

	if (sockFD < 0)
	{
		printf("\nFailed to create socket");
        return EXIT_FAILURE;
	}

	int status = setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	if (status < 0)
	{
		printf("\nFailed to set socket options");
		return EXIT_FAILURE;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	status = bind(sockFD, (struct sockaddr *)&address, sizeof(address));

	if ( status < 0)
	{
		printf("\nFailed to bind the address and port to the socket");
		return EXIT_FAILURE;
	}
	
	status = listen(sockFD, 5);

	if ( status < 0)
	{
		printf("\nFailed to listen on the socket");
		return EXIT_FAILURE;
	}

	printf("Server started successfully\n");

	int clientSocketFD = accept(sockFD, (struct sockaddr *)&address, (socklen_t *)&addrlen);
	if (clientSocketFD < 0)
	{
		printf("\nFailed to accept");
		return EXIT_FAILURE;
	}
	printf("Client connected successfully\n");
	saveFile(clientSocketFD);
	printf("Data saved to the file in sampleDataFromCLient.txt");

	return 0;
}