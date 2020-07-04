#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#define BUF_SIZE 4096

void resetBuffer(char givenBuffer[]);

void displayCommandExecuted(char* command);

void displayDataSent(char* sendBuffer);

int getFdSocketTcp(char* ip, char* port);

int getFdSocketUdp(int socketTcpFd);
