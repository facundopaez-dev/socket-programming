/* Al ejecutar el servidor se tiene proveer dos argumentos mediante la linea de comandos.
El primer argumento es la direccion IP 127.0.0.1 (dirección IP de loopback) y el segundo
argumento es el numero de un puerto que no sea ninguno de los puertos bien conocidos. */

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#define CONNECTION_LIMIT 1
#define BUF_SIZE 500 /* Maximum size of datagrams that can be read by client and server */

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

  /* Receive streams and return copies to senders */
  for (;;) {
    acceptfd = accept(socketfd, (struct sockaddr*) &addr, &addrlen);

    if (acceptfd == -1) {
      perror("Could not accept");
      exit(EXIT_FAILURE);
    }

    len = sizeof(struct sockaddr_storage);
    numRead = recvfrom(socketfd, buf, BUF_SIZE, 0, (struct sockaddr *) &claddr, &len);
    printf("%s\n", buf);

    if (numRead == -1) {
      fprintf(stderr, "%s\n", buf);
    }

    if (sendto(socketfd, buf, numRead, 0, (struct sockaddr *) &claddr, len) != numRead) {
      fprintf(stderr, "%s\n", buf);
    }

  } // End for

}
