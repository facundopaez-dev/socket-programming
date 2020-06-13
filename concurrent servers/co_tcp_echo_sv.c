#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include "commands.h"
#include "answers.h"
#define CONNECTION_LIMIT 10
#define BUF_SIZE 4096 /* Maximo tamaño del bufer para los flujos de datos que pueden leer el cliente y el servidor */

static void response(int acceptfd, char* answer) {

  if (write(acceptfd, answer, strlen(answer) + 1) >= 0) {
    printf("%s\n", answer);
  }

}

static void turnon(int acceptfd, const char* buf) {

  if (strcmp(buf, TURN_ON) == 0) {
    response(acceptfd, ANSWER_TURN_ON);
  }

}

static void turnoff(int acceptfd, const char* buf) {

  if (strcmp(buf, TURN_OFF) == 0) {
    response(acceptfd, ANSWER_TURN_OFF);
  }

}

static void ienable(int acceptfd, const char* buf) {

  if (strcmp(buf, I_ENABLE) == 0) {
    response(acceptfd, ANSWER_I_ENABLE);
  }

}

static void idisable(int acceptfd, const char* buf) {

  if (strcmp(buf, I_DISABLE) == 0) {
    response(acceptfd, ANSWER_I_DISABLE);
  }

}

static void rimage(int acceptfd, const char* buf) {

  if (strcmp(buf, R_IMAGE) == 0) {
    response(acceptfd, ANSWER_R_IMAGE);
  }

}

static void takecall(int acceptfd, const char* buf) {

  if (strcmp(buf, TAKE_CALL) == 0) {
    response(acceptfd, ANSWER_TAKE_CALL);
  }

}

static void disconnect(int acceptfd, const char* buf) {

  // Falta eliminar el fd del cliente del arreglo que contiene los fd de cada cliente conectado

  if (strcmp(buf, EXIT) == 0) {
    response(acceptfd, ANSWER_EXIT);
  }

}

static void ping(int acceptfd, const char* buf) {

  if (strcmp(buf, PING) == 0) {
    response(acceptfd, ANSWER_PING);
  }

}

/* Handle a client request: copy socket input back to socket */
static void handleRequest(int acceptfd) {
  char buf[BUF_SIZE];
  char returnBuf[BUF_SIZE];
  ssize_t numRead;

  while ((numRead = read(acceptfd, buf, BUF_SIZE)) > 0) {
    // Esto suprime el \n de la cadena enviada por el cliente
    buf[numRead - 1] = '\0';

    // Ejecucion de cada comando
    turnon(acceptfd, buf);
    turnoff(acceptfd, buf);
    ienable(acceptfd, buf);
    idisable(acceptfd, buf);
    rimage(acceptfd, buf);
    takecall(acceptfd, buf);
    disconnect(acceptfd, buf);
    ping(acceptfd, buf);

  } // End while

  if (numRead == -1) {
    perror("Error from read");
    exit(EXIT_FAILURE);
  }

}

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

  for (;;) {
    acceptfd = accept(socketfd, NULL, NULL); /* Wait for connection */

    if (acceptfd == -1) {
      perror("Accept error: Could not accept");
      exit(EXIT_FAILURE);
    }

    /* Handle each client request in a new child process */
    switch (fork()) {
      case -1:
      perror("Can't create child");
      close(acceptfd); /* Give up on this client */
      break; /* May be temporary; try next client */

      case 0: /* Child */
      close(socketfd); /* Unneeded copy of listening socket */
      handleRequest(acceptfd);
      exit(EXIT_SUCCESS);

      default: /* Parent */
      close(acceptfd); /* Unneeded copy of connected socket */
      break; /* Loop to accept next connection */
    }

  } // End for

}
