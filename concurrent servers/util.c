#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "utildefinitions.h"
#include "answers.h"

// TODO: Documentar lo que falte documentar

int sendResultClientUdp(int acceptfd, char* answer, int numRead, const struct sockaddr *claddr, socklen_t len) {
  return sendto(acceptfd, answer, numRead, 0, claddr, len);
}

int sendResultClient(int acceptfd, char* answer) {
  return write(acceptfd, answer, strlen(answer) + 1);
}

void sendResultServer(int acceptfd, char* command, char* answer, int resultWrite, int clients[]) {

  if (resultWrite >= 0) {
    int senderDepartmentId = getIdDepartment(clients, acceptfd);
    printf("The client of the department %i executes the command: %s\n", senderDepartmentId, command);
    printf("Response: %s\n", answer);
  }

}

void sendNoticeReceiver(int acceptfd, int receivingDepartmentId, char* notice) {
  if (write(acceptfd, notice, strlen(notice) + 1) >= 0) {
    printf("Notification for customer %i: %s\n", receivingDepartmentId, notice);
  }
}

void invalidCommand(int acceptfd, char* invalidCommand, int clients[]) {
  responseInvalidCommand(acceptfd, ANSWER_INVALID_COMMAND, invalidCommand, clients);
}

void responseInvalidCommand(int acceptfd, char* answer, char* invalidCommand, int clients[]) {

  if (write(acceptfd, answer, strlen(answer) + 1) >= 0) {
    int senderDepartmentId = getIdDepartment(clients, acceptfd);
    printf("The client of the department %i executes an invalid command\n", senderDepartmentId);
    printf("Invalid command: %s\n", invalidCommand);
    printf("Response: %s\n", answer);
  }

}

bool existDepartment(int idDepartment) {
  /*
   * El ID del departamento es el numero de la posicion, del arreglo
   * de conexiones, en la cual se encuentra el descriptor de
   * archivo devuelto por la llamada al sistema accept al conectarse
   * un cliente al servidor
   *
   * Sabiendo lo anterior se puede determinar si existe el ID de un departamento,
   * enviado por otro cliente, verificando que este dentro del tamaño establecido
   * para el arreglo de conexiones, tamaño que es determinado por el valor de la
   * constante CONNECTION_LIMIT
   *
   * La verificacion se hace a partir de 1 porque si se la hace a partir de 0
   * se obtendra el valor -1 cuando se decremente el valor de idDepartment = 0 por uno
   * para obtener el indice del arreglo de conexiones, lo cual se hace para obtener
   * el descriptor de archivo del cliente al que otro cliente le quiere enviar algo
   */
  if ((idDepartment >= 1) && (idDepartment <= CONNECTION_LIMIT)) {
    return true;
  }

  return false;
}

bool connectedClient(int idDepartment, int clients[]) {
  /*
   * Si el contenido de la posicion indicada por
   * idDepartment - 1 es distinto de FREE_CONNECTION (la cual
   * vale -1), entonces significa que el cliente solicitado
   * esta conectado al servidor, en este caso esta funcion
   * retorna verdadero, en caso contrario retorna falso
   */
  if (clients[idDepartment - 1] != FREE_CONNECTION) {
    return true;
  }

  return false;
}

bool equalDepartment(int senderfd, int idDepartment, int clients[]) {

  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {

    if ((clients[i] == senderfd) && (i == idDepartment - 1)) {
      return true;
    }

  }

  return false;
}

void printConnections(int *amountConnections) {
  printf("%s%i\n", "Ammount connections: ", *amountConnections);
}

/**
 * Busca el identificador (ID) del departamento (celda
 * del arreglo de conexiones) en el cual hay un cliente,
 * el cual esta representado por el descriptor de archivo
 * del socket con el que se establecio la conexion entre
 * un cliente y el servidor
 *
 * @param  clients
 * @param  acceptfd
 * @return ID del departamento del cliente asociado
 * al descriptor de archivo contenido en la variable
 * entera acceptfd
 */
int getIdDepartment(int clients[], int acceptfd) {

  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {
    if (clients[i] == acceptfd) {
      return i + 1;
    }
  }

}

/**
 * Elimina la basura que haya en el arreglo
 * de caracteres enviado como argumento
 *
 * @param buf [arreglo de caracteres]
 */
void resetCharArray(char buf[]) {
  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {
    buf[i] = '\0';
  }
}

/**
 * Registra las conexiones que hay entre los clientes y el servidor, si
 * el servidor no ha llegado a su limite de conexiones activas
 *
 * @param  clients           [arreglo de clientes o conexiones]
 * @param  connectionfd      [FD del socket usado para la conexion entre el servidor y un cliente]
 * @param  amountConnections [cantidad de conexiones activas]
 * @return cantidad de conexiones activas
 */

// TODO: Modificar la descripcion
void addClient(int connectionfd, int *amountConnections, pthread_mutex_t lock, int clients[]) {
  pthread_mutex_lock(&lock);

  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {

    /*
     * Si la celda i del arreglo de clientes o conexiones
     * esta libre, entonces se puede establecer la conexion
     * entre el servidor y un cliente, y se registra esta
     * conexion en la celda i del arreglo de clientes
     *
     * Una celda se considera libre si tiene el valor -1
     */
    if (clients[i] == FREE_CONNECTION) {
      /*
       * Registra el descriptor de archivo del socket
       * a traves del cual se hace la comunicacion
       * entre el hilo y un cliente
       */
      clients[i] = connectionfd;

      /*
       * Se incrementa la cantidad de conexiones activas
       */
      (*amountConnections) = (*amountConnections) + 1;

      /*
       * Una vez que se encontro un espacio libre en el arreglo
       * se tiene que cortar el ciclo
       */
      break;
    }

  } // End for

  pthread_mutex_unlock(&lock);
}

void removeClient(int clients[], int connectionfd, int *amountConnections) {

  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {

    if (clients[i] == connectionfd) {
      clients[i] = FREE_CONNECTION;
      (*amountConnections) = (*amountConnections) - 1;

      /*
       * Una vez que se encontro el FD de un cliente en el arreglo
       * se tiene que cortar el ciclo
       */
      break;
    }

  }

}

void concatenateTextNotification(char* notification, char* firstPartNotification, char* secondPartNotification, int senderDepartmentId) {
  char charIdDepartment[BUF_SIZE];

  /*
   * Elimina la basura que haya en estos arreglos de
   * caracteres, ya que al crearlos no son inicializados,
   * con lo cual es probable que contenga datos basura
   */
  resetCharArray(notification);
  resetCharArray(charIdDepartment);

  charIdDepartment[0] = convertIntToChar(senderDepartmentId);

  strcpy(notification, firstPartNotification);

  /*
   * Concatena el contenido de charIdDepartment
   * y secondPartNotification, en este orden, con
   * el contenido de firstPartNotification, y el
   * resultado de esta concatenacion lo almacena
   * en la variable notification
   */
  strcat(notification, charIdDepartment);
  strcat(notification, secondPartNotification);
}

/**
 * Carga el arreglo dado con el vañor -1 para indicar
 * que no hay clientes conectados al servidor, o en
 * otras palabras que no hay conexiones abiertas
 *
 * @param clients [int array]
 */

// TODO: Modificar descripcion, y añadir el signifcado de -1
void fill(int clients[]) {

  for (size_t j = 0; j < CONNECTION_LIMIT; j++) {
    clients[j] = -1;
  }

}

char convertIntToChar(int number) {
  return number + '0';
}

int getFdSocketTcp(char* ip, char* port) {
  int sockettcpfd;
  int bindtcpfd;
  int listentcpfd;

  socklen_t addrlentcp;
  struct sockaddr_in addrtcp;

  addrlentcp = sizeof(addrtcp);

  inet_aton(ip, &(addrtcp.sin_addr));
  addrtcp.sin_port = htons(atoi(port));
  addrtcp.sin_family = AF_INET; /* Constante que indica el uso de IPv4 */

  sockettcpfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockettcpfd == -1) {
    perror("Socket error: Could not create server socket");
    exit(EXIT_FAILURE);
  }

  int optval = 1;

  // Esto indica que el puerto esta marcado para reutilizar
  if(setsockopt(sockettcpfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  bindtcpfd = bind(sockettcpfd, (struct sockaddr*) &addrtcp, addrlentcp);

  if (bindtcpfd == -1) {
    perror("Bind error: Could not bind server socket");
    exit(EXIT_FAILURE);
  }

  listentcpfd = listen(sockettcpfd, CONNECTION_LIMIT);

  if (listentcpfd == -1) {
    perror("Could not listen");
    exit(EXIT_FAILURE);
  }

  return sockettcpfd;
}

int getFdSocketUdp(char* ip, char* port) {
  int socketudpfd;
  int bindudpfd;
  socklen_t addrlenudp;

  struct sockaddr_in addrudp;
  addrlenudp = sizeof(addrudp);

  inet_aton(ip, &(addrudp.sin_addr));
  addrudp.sin_port = htons(atoi(port));

  /* La constante AF_INET indica que la comunicacion entre los clientes
  y el servidor (ambos son aplicaciones) va a ser utilizando el protocolo IPv4 */
  addrudp.sin_family = AF_INET;

  /* La constante SOCK_DGRAM sirve para indicar que se va a crear un
  socket de datagramas (servicio no orientado a la conexion) */
  socketudpfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (socketudpfd == -1) {
    perror("Socket error: Could not create server socket");
    exit(EXIT_FAILURE);
  }

  bindudpfd = bind(socketudpfd, (struct sockaddr*) &addrudp, addrlenudp);

  if (bindudpfd == -1) {
    perror("Bind error: Could not bind server socket");
    exit(EXIT_FAILURE);
  }

  return socketudpfd;
}
