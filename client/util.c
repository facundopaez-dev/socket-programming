#include "headers/utildefinitions.h"

void resetBuffer(char givenBuffer[]) {

  for (size_t i = 0; i < strlen(givenBuffer); i++) {
    givenBuffer[i] = '\0';
  }

}

void displayDataSent(char* command, char* sendBuffer) {
  printf("[CLIENT] Command executed: %s\n", command);
  printf("[CLIENT] Send buffer content: %s\n", sendBuffer);
}

int getFdSocketTcp(char* ip, char* port) {
  int socketTcpFd;
  int connectResult;

  socklen_t addrTcpLen;
  struct sockaddr_in addrTcp;

  addrTcpLen = sizeof(addrTcp);

  inet_aton(ip, &(addrTcp.sin_addr));
  addrTcp.sin_port = htons(atoi(port));
  addrTcp.sin_family = AF_INET;

  /*
   * AF_INET y SOCK_STREAM indican que este socket
   * usa IPv4 y flujos respectivamente para hacer
   * la comunicacion entre un cliente y el servidor
   */
  socketTcpFd = socket(AF_INET, SOCK_STREAM, 0);

  if (socketTcpFd == -1) {
    perror("Socket error: Could not create server socket");
    exit(EXIT_FAILURE);
  }

  int optval = 1;

  // Esto indica que el puerto esta marcado para reutilizar
  if(setsockopt(socketTcpFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  connectResult = connect(socketTcpFd, (struct sockaddr*) &addrTcp, addrTcpLen);

  if (connectResult == -1) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  return socketTcpFd;
}

int getFdSocketUdp(char* ip, char* port) {
  int socketUdpFd;
  int bindUdpFd;

  struct sockaddr_in addrUdp;
  socklen_t addrlenUdp;

  addrlenUdp = sizeof(addrUdp);

  inet_aton(ip, &(addrUdp.sin_addr));
  addrUdp.sin_port = htons(atoi(port));

  /* La constante AF_INET indica que la comunicacion entre los clientes
  y el servidor (ambos son aplicaciones) va a ser utilizando el protocolo IPv4 */
  addrUdp.sin_family = AF_INET;

  /* La constante SOCK_DGRAM sirve para indicar que se va a crear un
  socket de datagramas (servicio no orientado a la conexion) */
  socketUdpFd = socket(AF_INET, SOCK_DGRAM, 0);

  if (socketUdpFd == -1) {
    perror("Socket error: Could not create server socket");
    exit(EXIT_FAILURE);
  }

  int optval = 1;

  // Esto indica que el puerto esta marcado para reutilizar
  if(setsockopt(socketUdpFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  // Este no es necesario
  // bindUdpFd = bind(socketUdpFd, (struct sockaddr*) &addrUdp, addrlenUdp);
  //
  // if (bindUdpFd == -1) {
  //   perror("Bind error: Could not bind server socket");
  //   exit(EXIT_FAILURE);
  // }

  return socketUdpFd;
}
