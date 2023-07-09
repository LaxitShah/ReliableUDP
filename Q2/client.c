#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#define SIZE 2000

#define PORT 8090

void uploadFile(FILE *fp, int sockFD)
{
    char value[SIZE] = {'\0'};

    while (fgets(value, SIZE, fp) != NULL)
    {
        printf("\n%s", value);
        if (send(sockFD, value, sizeof(value), 0) == -1)
        {
            printf("\nFailed to send the file");
            return;
        }
        memset(value, 0, SIZE);
    }
}

int main()
{
    int sockFD;
    struct sockaddr_in serverAddress;
    FILE *fp;
    char *filename = "sampleData.txt";

    sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFD < 0)
    {
        printf("\nFailed to create socket");
        return EXIT_FAILURE;
    }
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    char *ipAddress = "127.0.0.1";
    serverAddress.sin_addr.s_addr = inet_addr(ipAddress);

    int clientFD;
    clientFD = connect(sockFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (clientFD == -1)
    {
        printf("\nFailed to connect to the server");
        return EXIT_FAILURE;
    }
    printf("\nConnected to server");

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("\nFailed to read a file");
        return EXIT_FAILURE;
    }

    uploadFile(fp, sockFD);
    printf("\nSuccessfully sent the file data");

    return 0;
}