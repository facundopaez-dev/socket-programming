#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#define BUF_SIZE 4096

void resetBuffer(char givenBuffer[]);

void displayDataSent(char* command, char* sendBuffer);

int getFdSocketTcp(char* ip, char* port);

int getFdSocketUdp(char* ip, char* port);
