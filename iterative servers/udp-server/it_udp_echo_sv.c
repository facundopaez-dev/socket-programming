/* Al ejecutar el servidor se tiene proveer dos argumentos mediante la linea de comandos.
El primer argumento es la direccion IP 127.0.0.1 (dirección IP de loopback) y el segundo
argumento es el numero de un puerto que no sea ninguno de los puertos bien conocidos. */

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#define BUF_SIZE 500 /* Tamaño maximo de los datagramas que pueden ser leidos por el cliente y el servidor */

int main(int argc, char *argv[]) {
  int socketfd;
  int bindfd;
  ssize_t numRead;
  socklen_t addrlen, len;
  char buf[BUF_SIZE];

  struct sockaddr_storage claddr;
  struct sockaddr_in addr;

  addrlen = sizeof(addr);

  inet_aton(argv[1], &(addr.sin_addr));
  addr.sin_port = htons(atoi(argv[2]));

  /* La constante AF_INET indica que la comunicacion entre los clientes
  y el servidor (ambos son aplicaciones) va a ser utilizando el protocolo IPv4 */
  addr.sin_family = AF_INET;

  /* La constante SOCK_DGRAM sirve para indicar que se va a crear un
  socket de datagramas (servicio no orientado a la conexion) */
  socketfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (socketfd == -1) {
    perror("Socket error: Could not create server socket");
    exit(EXIT_FAILURE);
  }

  bindfd = bind(socketfd, (struct sockaddr*) &addr, addrlen);

  if (bindfd == -1) {
    perror("Bind error: Could not bind server socket");
    exit(EXIT_FAILURE);
  }

  /* Recibe datagramas y retorna copias a los emisores */
  for (;;) {
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
