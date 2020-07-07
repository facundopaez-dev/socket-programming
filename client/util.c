#include "headers/utildefinitions.h"

/**
 * Coloca en cada posicion del buffer enviado
 * como argumento, el caracter '\0' para eliminar
 * cualquier dato basura
 *
 * @param givenBuffer
 */
void resetBuffer(char givenBuffer[]) {

  for (size_t i = 0; i < strlen(givenBuffer); i++) {
    givenBuffer[i] = '\0';
  }

}

/**
 * Muestra por pantalla el comando que fue ejecutado
 * por el usuario, comando que es pasado por argumento
 * a esta funcion cuando es invocada
 *
 * @param command
 */
void displayCommandExecuted(char* command) {
  printf("[CLIENT] Command executed: %s\n", command);
}

/**
 * Muestra el contenido de lo que el programa cliente
 * le envia al servidor (con el cual esta conectado)
 * cuando el usuario introduce un comando valido
 *
 * @param sendBuffer
 */
void displayDataSent(char* sendBuffer) {
  printf("[CLIENT] Send buffer content: %s\n", sendBuffer);
}

/**
 * Esta funcion tiene la responsabilidad crear un socket
 * TCP haciendo uso de una IP y un puerto
 *
 * @param  ip
 * @param  port
 * @return el descriptor de archivo del socket TCP creado
 * con la IP y puerto pasados como argumento en la invocacion
 * de esta funcion
 */
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

/*
 * Recibir IP y puerto como numero, o va
 * a tener q recibir el socket TCP para averiguar
 * IP y puerto locales
 *
 * Esto se lo tiene que hacer despues de ejecutar connect()
 */

/**
 * Esta funcion tiene la responsabilidad de crear un socket
 * UDP haciendo uso de la IP y el puerto asociados al
 * descriptor de archivo de un socket TCP
 *
 * @param  socketTcpFd
 * @return el descriptor de archivo del socket UDP creado
 * con la IP y el puerto asociado al descriptor de archivo
 * de un socket TCP           
 */
int getFdSocketUdp(int socketTcpFd) {
  int socketUdpFd;
  int bindUdpFd;

  struct sockaddr_in addrUdp;
  socklen_t addrlenUdp = sizeof(struct sockaddr_in);

  getsockname(socketTcpFd, (struct sockaddr *) &addrUdp, &addrlenUdp);

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

  bindUdpFd = bind(socketUdpFd, (struct sockaddr*) &addrUdp, addrlenUdp);

  // printf("[CLIENT][UDP] Se ejecuta el bind()\n", "");
  // printf("[CLIENT] Port: %d\n", ntohs(addrUdp.sin_port));
  // printf("[CLIENT] IP adress: %s\n", inet_ntoa(addrUdp.sin_addr));

  if (bindUdpFd == -1) {
    perror("Bind error: Could not bind server socket");
    exit(EXIT_FAILURE);
  }

  return socketUdpFd;
}
