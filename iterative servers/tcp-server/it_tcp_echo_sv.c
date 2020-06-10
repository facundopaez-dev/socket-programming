/* Al ejecutar el servidor se tiene proveer dos argumentos mediante la linea de comandos.
El primer argumento es la direccion IP 127.0.0.1 (dirección IP de loopback) y el segundo
argumento es el numero de un puerto que no sea ninguno de los puertos bien conocidos. */

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#define CONNECTION_LIMIT 1
#define BUF_SIZE 500 /* Maximo tamaño del bufer para los flujos de datos que pueden leer el cliente y el servidor */

int main(int argc, char *argv[]) {
  int socketfd;
  int bindfd;
  int listenfd;
  int acceptfd;

  ssize_t numRead;
  socklen_t addrlen, len;
  char buf[BUF_SIZE];

  struct sockaddr_storage claddr;
  struct sockaddr_in addr;

  addrlen = sizeof(addr);

  inet_aton(argv[1], &(addr.sin_addr));
  addr.sin_port = htons(atoi(argv[2]));
  addr.sin_family = AF_INET; /* Constante que indica el uso de IPv4 */

  socketfd = socket(AF_INET, SOCK_STREAM, 0);

  if (socketfd == -1) {
    perror("Socket error: Could not create server socket");
    exit(EXIT_FAILURE);
  }

  int optval = 1;

  // Esto indica que el puerto esta marcado para reutilizar
  if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  bindfd = bind(socketfd, (struct sockaddr*) &addr, addrlen);

  if (bindfd == -1) {
    perror("Bind error: Could not bind server socket");
    exit(EXIT_FAILURE);
  }

  listenfd = listen(socketfd, CONNECTION_LIMIT);

  if (listenfd == -1) {
    perror("Could not listen");
    exit(EXIT_FAILURE);
  }

  acceptfd = accept(socketfd, (struct sockaddr*) &addr, &addrlen);

  if (acceptfd == -1) {
    perror("Accept error: Could not accept");
    exit(EXIT_FAILURE);
  }

  /* Recibe flujos de datos y retorna copia a los emisores */
  for (;;) {
    len = sizeof(struct sockaddr_storage);
    numRead = recv(acceptfd, buf, BUF_SIZE, 0);
    buf[numRead] = '\0';

    printf("%s\n", buf);

    if (numRead == -1) {
      fprintf(stderr, "%s\n", buf);
    }

    if (send(acceptfd, buf, numRead, 0) != numRead) {
      fprintf(stderr, "%s\n", buf);
    }

  } // End for

}
