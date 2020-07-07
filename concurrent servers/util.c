#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>

// Headers
#include "headers/answers.h"
#include "headers/utildefinitions.h"

/**
 * Esta funcion tiene la responsabilidad de enviarle a un cliente
 * el resultado del comando que haya ejecutado mediante UDP
 *
 * @param  fd     [descriptor de archivo de un socket UDP]
 * @param  answer [buffer que contiene la respuesta de ejecutar un comando]
 * @return el resultado de la instruccion sendto
 */
int sendResultClientUdp(int fd, char* answer) {
  socklen_t len = sizeof(struct sockaddr_in);
  struct sockaddr_in addr;

  int resultSend = sendto(fd, answer, BUF_SIZE, 0, (struct sockaddr *) &addr, len);

  if (resultSend == -1) {
    perror("send");
    exit(EXIT_FAILURE);
  }

  return resultSend;
}

/**
 * Esta funcion tiene la responsabilidad de enviarle a un cliente
 * el resultado del comando que haya ejecutado mediante TCP
 *
 * @param  acceptfd [descriptor de archivo de un socket TCP]
 * @param  answer   [buffer que contiene la respuesta de ejecutar un comando]
 * @return el resultado de la instruccion write
 */
int sendResultClient(int acceptfd, char* answer) {
  return write(acceptfd, answer, strlen(answer) + 1);
}

/**
 * Esta funcion tiene la responsabilidad de enviarle a un cliente
 * el resultado del comando que haya ejecutado mediante UDP
 *
 * @param  senderUdpFd [descriptor de archivo de un socket UDP]
 * @param  answer      [buffer que contiene la respuesta de ejecutar un comando]
 * @param  addrSender  [estructura que contiene la IP y el puerto de un cliente que usa UDP]
 * @return el resultado de la instruccion sendto
 */
int sendNoticeClientUdp(int senderUdpFd, char* answer, struct sockaddr_in addrSender) {
  return sendto(senderUdpFd, answer, BUF_SIZE, 0, (struct sockaddr *) &addrSender, sizeof(struct sockaddr_in));
}

/**
 * Esta funcion tiene la responsabilidad de mostrar en la pantalla
 * del servidor el resultado de ejecutar un comando, dicho resultado
 * tiene: El ID del departamento del cliente que ejecuto un comando,
 * el nombre del comando ejecutado por dicho cliente y la respuesta
 * de parte del servidor hacia dicho cliente
 *
 * @param acceptfd    [descriptor de archivo de un socket TCP]
 * @param command     [buffer que contiene el nombre del comando ejecutado por un cliente]
 * @param answer      [buffer que contiene la respuesta de haber ejecutado un comando]
 * @param resultWrite [variable que contiene el resultado de una instruccion write]
 * @param clients     [arreglo que contiene el descriptor de archivo de cada socket TCP usado para las conexiones de los clientes]
 */
void sendResultServer(int acceptfd, char* command, char* answer, int resultWrite, int clients[]) {

  if (resultWrite > 0) {
    int senderDepartmentId = getIdDepartment(clients, acceptfd);
    printf("[SERVER] The client of the department %i executes the command: %s\n", senderDepartmentId, command);
    printf("[SERVER] Response to client: %s\n", answer);
  }

}

/**
 * Esta funcion tiene la responsabilidad de mostrar en la pantalla
 * del servidor el resultado de ejecutar un comando, dicho resultado
 * tiene: El ID del departamento del cliente que ejecuto un comando,
 * el nombre del comando ejecutado por dicho cliente y la respuesta
 * de parte del servidor hacia dicho cliente
 *
 * @param senderDepartmentId [ID del departamento del cliente que ejecuto un comando]
 * @param command            [buffer que contiene el nombre del comando ejecutado por un cliente]
 * @param answer             [buffer que contiene la respuesta de haber ejecutado un comando]
 * @param operationResult    [variable que contiene el resultado de ejecutar una instruccion write o sendto]
 */
void sendReplyServer(int senderDepartmentId, char* command, char* answer, int operationResult) {

  if (operationResult > 0) {
    printf("[SERVER] The client of the department %i executes the command: %s\n", senderDepartmentId, command);
    printf("[SERVER] Response to client: %s\n", answer);
  }

}

/**
 * Esta funcion tiene la responsabilidad de enviarle una notificacion
 * al cliente receptor indicandole que un cliente emisor realizo una
 * acccion sobre el (como una llamada, por ejemplo)
 *
 * @param acceptfd              [descriptor de archivo de un socket TCP]
 * @param receivingDepartmentId [ID del departamento del cliente receptor]
 * @param notice                [buffer que contiene la notificacion]
 */
void sendNoticeReceiver(int acceptfd, int receivingDepartmentId, char* notice) {
  if (write(acceptfd, notice, strlen(notice) + 1) >= 0) {
    printf("[SERVER] Notification for customer %i: %s\n", receivingDepartmentId, notice);
  }
}

void invalidCommand(int acceptfd, int clients[]) {
  responseInvalidCommand(acceptfd, clients);
}

/**
 * Esta funcion tiene la responsabilidad de mostrar en
 * la pantalla del servidor la ejecucion de un comando
 * invalido
 *
 * @param acceptfd [descriptor de archivo de un socket TCP]
 * @param clients  [arreglo que contiene el descriptor de archivo de cada socket TCP usado para las conexiones de los clientes]
 */
void responseInvalidCommand(int acceptfd, int clients[]) {

  if (write(acceptfd, ANSWER_INVALID_COMMAND, BUF_SIZE) >= 0) {
    int senderDepartmentId = getIdDepartment(clients, acceptfd);
    printf("[SERVER] The client of the department %i executes an invalid command\n", senderDepartmentId);
    printf("[SERVER] Response to client: %s\n", ANSWER_INVALID_COMMAND);
  }

}

/**
 * Esta funcion tiene la responsabilidad de comprobar si
 * un departamento existe o no, lo cual se hace comprobando
 * si el ID (provisto por un cliente) del dapartamento esta
 * entre 1 y el limite de conexiones del servidor
 *
 * @param  idDepartment [ID del departamento provisto por un cliente]
 * @return verdadero en caso de que existe el departamento, en caso
 * constrario falso
 */
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

/**
 * Esta funcion tiene la responsabilidad de comprobar si un
 * cliente esta conectado al servidor, para esto hace uso
 * del ID de un dapartamento
 *
 * Si en la posicion indicada por ID - 1 en el arreglo clients
 * hay un numero distintos de -1 es porque hay un cliente conectado
 * en el departamento que tiene el ID dado
 *
 * Si en la posicion indicada por ID - 1 en el arreglo clients
 * hay un numero igual a -1 es porque no hay un cliente conectado
 * en el departamento que tiene el ID dado
 *
 * @param  idDepartment [ID de un departamento]
 * @param  clients      [arreglo que contiene el descriptor de archivo de cada socket TCP usado para las conexiones de los cliente]
 * @return verdadero si el cliente esta conectado, en caso
 * contrario falso
 */
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

/**
 * Esta funcion tiene la responsabilidad de comprobar si
 * el ID (provisto por un cliente) de un departamento es
 * igual al ID del departamento en el cual se encuentra
 * conectado el cliente que provee el ID
 *
 * @param  givenSocketFd [descriptor de archivo de un socket TCP]
 * @param  idDepartment  [ID de un departamento provisto por un cliente]
 * @param  arrayClients  [arreglo que contiene el descriptor de archivo, sea de TCP o UDP, de cada cliente conectado al servidor]
 * @return verdadero si ID del departamento es igual al ID del departamento
 * que proveyo el ID, en caso contrario falso
 */
bool equalDepartment(int givenSocketFd, int idDepartment, int arrayClients[]) {

  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {

    /*
     * Comprueba si el descriptor de archivo dado (perteneciente
     * a un cliente) esta en el arreglo de clientes y si lo esta
     * comprueba que la posicion en la cual se encuentre el
     * descriptor de archivo sea igual al ID del departamento
     * provisto por un cliente
     */
    if ((arrayClients[i] == givenSocketFd) && (i == idDepartment - 1)) {
      return true;
    }

  }

  return false;
}

void printConnections(int *amountConnections) {
  printf("%s%i\n", "[SERVER] Ammount connections: ", *amountConnections);
}

/**
 * Busca el identificador (ID) del departamento (celda
 * del arreglo de conexiones) en el cual hay un cliente,
 * el cual esta representado por el descriptor de archivo
 * del socket con el que se establecio la conexion entre
 * un cliente y el servidor
 *
 * @param  clients  [arreglo que contiene el descriptor de archivo de cada socket TCP usado para las conexiones de los cliente]
 * @param  acceptfd [descriptor de archivo de un socket TCP]
 * @return ID del departamento del cliente asociado
 * al descriptor de archivo contenido en la variable
 * entera acceptfd
 */
int getIdDepartment(int clients[], int acceptfd) {
  /*
   * Comprueba si en la posicion i, del arreglo que contiene
   * los descriptores de archivo TCP de los clientes, esta
   * el descriptor de archivo del socket TCP provisto como
   * argumento, si es asi se suma 1 al valor de i, retornando
   * el ID del departamento en el cual se encuentra el cliente
   * que tiene el descriptor de archivo TCP dado
   */
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
  for (size_t i = 0; i < strlen(buf); i++) {
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
/**
 * Esta funcion tiene la responsabilidad de registrar en los arreglos de
 * descriptores de archivo TCP y UDP, el descriptor de archivo TCP y UDP
 * de cada cliente que se conecta a este servidor
 *
 * @param acceptFd          [descriptor de archivo de un socket TCP]
 * @param socketUdpFd       [descriptor de archivo de un socket UDP]
 * @param amountConnections [variable que representa la cantidad de clientes conectados que tiene el servidor]
 * @param lock              [variable utilizada para la exclusion mutua de los recursos compartidos por los hilos]
 * @param clients           [arreglo que contiene el descriptor de archivo de cada socket TCP usado para las conexiones de los client]
 * @param clientsUdp        [arreglo que contiene el descriptor de archivo de cada socket UDP usado para las conexiones de los client]
 */
void addClient(int acceptFd, int socketUdpFd, int *amountConnections, pthread_mutex_t lock, int clients[], int clientsUdp[]) {
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
      clients[i] = acceptFd;
      clientsUdp[i] = socketUdpFd;

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

/**
 * Esta funcion tiene la responsabilidad de eliminar los descriptores
 * de archivo TCP y UDP (de los arreglos clients y clientsUdp) del cliente
 * que ha ejecutado el comando exit, el cual es para desconectarse del servidor
 *
 * @param clients           [arreglo que contiene el descriptor de archivo de cada socket TCP usado para las conexiones de los client]
 * @param clientsUdp        [arreglo que contiene el descriptor de archivo de cada socket UDP usado para las conexiones de los client]
 * @param connectionfd      [descriptor de archivo TCP de un cliente que se ha desconectado del servidor]
 * @param amountConnections [cantidad de clientes conectados a este servidor]
 */
void removeClient(int clients[], int clientsUdp[], int connectionfd, int *amountConnections) {

  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {

    /*
     * Comprueba si en la posicion i del arreglo que
     * contiene los descriptores de archivo TCP esta
     * el descriptor de archivo dado, si es asi se
     * coloca en la posicion i de los arreglos
     * clients y clientsUdp el valor -1, el cual
     * esta contenido en la constante FREE_CONNECTION
     * y representa que en el departamento i no
     * hay un cliente conectado
     */
    if (clients[i] == connectionfd) {
      clients[i] = FREE_CONNECTION;
      clientsUdp[i] == FREE_CONNECTION;

      /*
       * Se decrementa la cantidad de conexiones que hay
       * en este servidor
       */
      (*amountConnections) = (*amountConnections) - 1;

      /*
       * Una vez que se encontro el FD de un cliente en el arreglo
       * se tiene que cortar el ciclo
       */
      break;
    }

  }

}

/**
 * Esta funcion tiene la responsabilidad de armar la notificacion
 * con las cadenas de caracteres que se pasan como argumento y el
 * ID del departamento del cliente emisor
 *
 * Esta notificacion es para el cliente receptor
 *
 * @param notification           [buffer que contiene la notificacion]
 * @param firstPartNotification  [buffer que contiene la primera parte de la notificacion]
 * @param secondPartNotification [buffer que contiene la segunda parte de la notificacion]
 * @param senderDepartmentId     [ID del departamento del cliente emisor]
 */
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
 * Esta funcion tiene la responsabilidad de cargar el arreglo
 * que se le pasa como argumento con el valor -1, el cual
 * representa que no hay un cliente conectado
 *
 * @param arrayClients [arreglo que contiene el descriptor de archivo, sea de TCP o UDP, de cada cliente conectado al servidor]
 */
void fill(int arrayClients[]) {

  for (size_t j = 0; j < CONNECTION_LIMIT; j++) {
    arrayClients[j] = -1;
  }

}

char convertIntToChar(int number) {
  return number + '0';
}

/**
 * Esta funcion tiene la responsabilidad de crear un socket TCP
 * dado una IP y un puerto
 *
 * @param  ip
 * @param  port
 * @return el descriptor de archivo del socket TCP creado
 */
int getFdSocketTcp(char* ip, char* port) {
  int sockettcpfd;
  int bindtcpfd;
  int listentcpfd;

  socklen_t addrlentcp;
  struct sockaddr_in addrtcp;

  addrlentcp = sizeof(addrtcp);

  inet_aton(ip, &(addrtcp.sin_addr));
  addrtcp.sin_port = htons(atoi(port));
  addrtcp.sin_family = AF_INET;

  /*
   * AF_INET y SOCK_STREAM indican que este socket
   * usa IPv4 y flujos respectivamente para hacer
   * la comunicacion entre un cliente y el servidor
   */
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

/**
 * Esta funcion tiene la responsabilidad de crear un socket UDP
 * usando el descriptor de archivo de un socket TCP
 *
 * @param  acceptfd [descriptor de archivo de un socket TCP]
 * @return el descriptor de archivo de un socket UDP
 */
int getFdSocketUdp(int acceptfd) {
  int socketudpfd;
  int bindudpfd;

  struct sockaddr_in sudpaddr;
  socklen_t udpaddrlen = sizeof(sudpaddr);

  /*
   * La funcion getsockname carga una struct sockaddr
   * con la IP y el puerto del servidor
   */
  getsockname(acceptfd, (struct sockaddr *) &sudpaddr, &udpaddrlen);

  // Puerto efimero
  sudpaddr.sin_port = 0;

  /*
   * AF_INET y SOCK_DGRAM indican que este socket
   * usa IPv4 y datagramas respectivamente para hacer
   * la comunicacion entre un cliente y un servidor
   */
  socketudpfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (socketudpfd == -1) {
    perror("Socket error: Could not create server socket");
    exit(EXIT_FAILURE);
  }

  int optval = 1;

  // Esto indica que el puerto esta marcado para reutilizar
  if(setsockopt(socketudpfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  bindudpfd = bind(socketudpfd, (struct sockaddr*) &sudpaddr, udpaddrlen);

  if (bindudpfd == -1) {
    perror("Bind error: Could not bind server socket");
    exit(EXIT_FAILURE);
  }

  return socketudpfd;
}

/**
 * @param  givenSocketFd
 * @return los datos (IP y puerto) del servidor asociados
 * al descriptor de archivo de un socket
 */
struct sockaddr_in getDataServer(int givenSocketFd) {
  struct sockaddr_in addrServer;
  socklen_t len = sizeof(struct sockaddr_in);

  /*
   * Obtiene los datos del servidor asociados al descriptor
   * de archivo (de un socket) pasado como argumento
   */
  int resultGetSockName = getsockname(givenSocketFd, (struct sockaddr *) &addrServer, &len);

  if (resultGetSockName == -1) {
    perror("getsockname");
    exit(EXIT_FAILURE);
  }

  return addrServer;
}
