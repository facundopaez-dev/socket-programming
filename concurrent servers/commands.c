#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

// Headers
#include "headers/answers.h"
#include "headers/commandsdefinitions.h"
#include "headers/confirmations.h"
#include "headers/namecommands.h"
#include "headers/notices.h"
#include "headers/utildefinitions.h"

// Constants
#define FILE_NAME "M101_hires_STScI-PRC2006-10a.jpg"

/**
 * Esta funcion tiene como responsabilidad simular el
 * encendido de las luces
 *
 * @param acceptfd           [descriptor de archivo de un socket TCP]
 * @param clients            [arreglo que contiene los descriptores de archivo de los sockets TCP usados para las conexiones de los clientes con este servidor]
 */
void turnon(int acceptfd, int clients[]) {
  /*
   * El servidor le envia al cliente que ejecuto el comando
   * turnon la respuesta "light on", la cual esta contenida
   * en la constante ANSWER_SENDER_TURN_ON
   */
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_TURN_ON);

  /*
   * Se muestra en la pantalla del servidor, los datos asociados
   * a la ejecucion del comando turnon, los cuales son: El nombre
   * del comando, el ID del departamento del cliente que ejecuto
   * el comando y la respuesta del servidor al cliente que ejecuto
   * el comando turnon
   */
  sendResultServer(acceptfd, TURN_ON, ANSWER_SENDER_TURN_ON, resultWrite, clients);
}

/**
 * Esta funcion tiene como responsabilidad simular el
 * apagado de las luces
 *
 * @param acceptfd           [descriptor de archivo de un socket TCP]
 * @param clients            [arreglo que contiene los descriptores de archivo de los sockets TCP usados para las conexiones de los clientes con este servidor]
 */
void turnoff(int acceptfd, int clients[]) {
  /*
   * El servidor le envia al cliente que ejecuto el comando
   * turnoff la respuesta "light off", la cual esta
   * contenida en la constante ANSWER_SENDER_TURN_OFF
   */
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_TURN_OFF);

  /*
   * Se muestra en la pantalla del servidor, los datos asociados
   * a la ejecucion del comando turnoff, los cuales son: El nombre
   * del comando, el ID del departamento del cliente que ejecuto
   * el comando y la respuesta del servidor al cliente que ejecuto
   * el comando turnoff
   */
  sendResultServer(acceptfd, TURN_OFF, ANSWER_SENDER_TURN_OFF, resultWrite, clients);
}

/**
 * Esta funcion tiene como responsabilidad simular la
 * activacion del riego automatico
 *
 * @param acceptfd           [descriptor de archivo de un socket TCP]
 * @param clients            [arreglo que contiene los descriptores de archivo de los sockets TCP usados para las conexiones de los clientes con este servidor]
 */
void ienable(int acceptfd, int clients[]) {
  /*
   * El servidor le envia al cliente que ejecuto el comando
   * ienable la respuesta "irrigation enabled", la cual
   * esta contenida en la constante ANSWER_SENDER_I_ENABLE
   */
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_I_ENABLE);

  /*
   * Se muestra en la pantalla del servidor, los datos asociados
   * a la ejecucion del comando ienable, los cuales son: El nombre
   * del comando, el ID del departamento del cliente que ejecuto
   * el comando y la respuesta del servidor al cliente que ejecuto
   * el comando ienable
   */
  sendResultServer(acceptfd, I_ENABLE, ANSWER_SENDER_I_ENABLE, resultWrite, clients);
}

/**
 * Esta funcion tiene como responsabilidad simular la
 * desactivacion del riego automatico
 *
 * @param acceptfd           [descriptor de archivo de un socket TCP]
 * @param clients            [arreglo que contiene los descriptores de archivo de los sockets TCP usados para las conexiones de los clientes con este servidor]
 */
void idisable(int acceptfd, int clients[]) {
  /*
   * El servidor le envia al cliente que ejecuto el comando
   * idisable la respuesta "irrigation disabled", la cual
   * esta contenida en la constante ANSWER_SENDER_I_DISABLE
   */
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_I_DISABLE);

  /*
   * Se muestra en la pantalla del servidor, los datos asociados
   * a la ejecucion del comando idisable, los cuales son: El nombre
   * del comando, el ID del departamento del cliente que ejecuto
   * el comando y la respuesta del servidor al cliente que ejecuto
   * el comando idisable
   */
  sendResultServer(acceptfd, I_DISABLE, ANSWER_SENDER_I_DISABLE, resultWrite, clients);
}

/**
 * Esta funcion tiene la responsabilidad de transferir un archivo
 * desde este servidor a un cliente
 *
 * @param acceptfd           [descriptor de archivo de un socket TCP]
 * @param clients            [arreglo que contiene los descriptores de archivo de los sockets TCP usados para las conexiones de los clientes con este servidor]
 */
void rimage(int acceptFd, int clients[]) {
  int numWrite;
  int fileFd;
  int resultSend;
  int bytesSended = 1;
  int resultSendFile;

  off_t fileSize;
  struct stat fileInfo;

  /*
   * El servidor le envia al cliente, que ejecuto el comando
   * rimage, el aviso de que se le quiere enviar un archivo
   */
  numWrite = write(acceptFd, S_IMAGE, BUF_SIZE);

  if (numWrite == -1) {
    perror("write");
    exit(EXIT_FAILURE);
  }

  fileFd = open(FILE_NAME, O_RDONLY);

  /*
   * El bloque then se ejecuta si en el directorio
   * del servidor hay un archivo que tiene el nombre
   * dado por la constante FILE_NAME, y por ende,
   * se realiza la transferencia de dicho archivo
   * desde este servidor a un cliente
   */
  if (fileFd > -1) {
    /*
     * Si el flujo de ejecucion de la funcion llega hasta
     * aca es porque el arhivo que el servidor desea abrir
     * existe en su directorio
     */
    printf("[SERVER] Open image file\n", "");

    /*
     * El servidor le confirma al cliente, que ejecuto
     * el comando rimage, que existe un archivo para
     * la transferencia
     */
    numWrite = write(acceptFd, EXISTING_FILE, BUF_SIZE);

    if (numWrite == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    }

    /*
     * Obtiene la informacion del archivo asociado
     * al descriptor de archivo que se pasa como
     * primer argumento, y coloca dicha informacion
     * en la estructura fileInfo
     */
    fstat(fileFd, &fileInfo);
    fileSize = htonl(fileInfo.st_size);
    printf("[SERVER] File size: %d\n", fileInfo.st_size);

    /*
     * El servidor le envia al cliente, que ejecuto el comando
     * rimage, el tamaño del archivo abierto
     */
    resultSend = send(acceptFd, (void *) &fileSize, sizeof(fileSize), 0);

    if (resultSend == -1) {
      perror("send");
      exit(EXIT_FAILURE);
    }

    /*
     * Esta parte del codigo fuente se encarga de la
     * transferencia del archivo
     */
    while (bytesSended > 0) {
      /*
       * Cuando no hayan mas bytes para leer del
       * archivo resultSendFile tendra el valor 0, el
       * cual asignara a la bytesSended, lo cual a
       * su vez hace que la instruccion while termine
       * de ejecutarse
       */
      resultSendFile = sendfile(acceptFd, fileFd, NULL, BUF_SIZE);

      if (resultSendFile == -1) {
        perror("sendfile");
        exit(EXIT_FAILURE);
      }

      bytesSended = resultSendFile;
    }

    int clientDepartmentId = getIdDepartment(clients, acceptFd);
    printf("[SERVER] The client of the department %i executes the command: %s\n", clientDepartmentId, R_IMAGE);
    printf("[SERVER] Response to client: %s", EXISTING_FILE);
    printf("%s\n", "");
    printf("[SERVER] File transfer completed\n", "");
    printf("%s\n", "");
  } // End if

  /*
   * El bloque then se ejecuta en caso de que no exista
   * un archivo en el directorio del servidor
   * con el nombre dado por la constante FILE_NAME, y
   * por ende, no se realiza la transferencia de
   * dicho archivo desde este servidor a un cliente
   */
  if (fileFd == -1) {
    /*
     * El servidor le confirma al cliente, que ejecuto
     * el comando rimage, que no existe un archivo para
     * la transferencia
     */
    numWrite = write(acceptFd, NONEXISTING_FILE, BUF_SIZE);

    if (numWrite == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    }

    int clientDepartmentId = getIdDepartment(clients, acceptFd);
    printf("[SERVER] The client of the department %i executes the command: %s\n", clientDepartmentId, R_IMAGE);
    perror("[SERVER] open");
    printf("[SERVER] Response to client: %s\n", NONEXISTING_FILE);
    printf("%s\n", "");
  }

}

/**
 * Esta funcion tiene la responsabilidad de simular la
 * recepcion de una llamada para aquel cliente que
 * que ejecuta el comando takecall
 *
 * @param acceptfd           [descriptor de archivo de un socket TCP]
 * @param clients            [arreglo que contiene los descriptores de archivo de los sockets TCP usados para las conexiones de los clientes con este servidor]
 */
void takecall(int acceptfd, int clients[]) {
  /*
   * El servidor le envia al cliente que ejecuto el comando
   * takecall la respuesta "call taken", la cual esta
   * contenida en la constante ANSWER_RECEIVER_TAKE_CALL
   */
  int resultWrite = sendResultClient(acceptfd, ANSWER_RECEIVER_TAKE_CALL);

  /*
   * Se muestra en la pantalla del servidor, los datos asociados
   * a la ejecucion del comando takecall, los cuales son: El nombre
   * del comando, el ID del departamento del cliente que ejecuto
   * el comando y la respuesta del servidor al cliente que ejecuto
   * el comando takecall
   */
  sendResultServer(acceptfd, TAKE_CALL, ANSWER_RECEIVER_TAKE_CALL, resultWrite, clients);
}

/**
 * Esta funcion tiene la responsabilidad de simular una
 * llamada de un cliente a otro cliente, cuando en uno de ellos
 * se ejecuta el comando callto seguido del ID del departamento
 * del cliente receptor de la llamada
 *
 * @param senderfd            [descriptor de archivo del socket TCP del cliente que inicia la llamada]
 * @param destinyDepartmentId [ID del departamento del cliente receptor de la llamada]
 * @param clients            [arreglo que contiene los descriptores de archivo de los sockets TCP usados para las conexiones de los clientes con este servidor]
 */
void callto(int senderfd, int destinyDepartmentId, int clients[]) {
  int resultWrite;

  /*
   * Comprueba que el ID del departamento recibido como
   * argumento no exista, si no existe, se retorna el
   * control a la funcion que invoco a callto
   */
  if (!existDepartment(destinyDepartmentId)) {
    resultWrite = sendResultClient(senderfd, ANSWER_NO_DEPARTMENT);
    sendResultServer(senderfd, CALL_TO, ANSWER_NO_DEPARTMENT, resultWrite, clients);
    return;
  }

  /*
   * Comprueba que el ID del departamento recibido como
   * argumento sea igual al ID del departamento del cliente
   * que quiere iniciar la llamada, si es asi, se retorna el
   * control a la funcion que invoco a callto
   */
  if (equalDepartment(senderfd, destinyDepartmentId, clients)) {
    resultWrite = sendResultClient(senderfd, ANSWER_EQUAL_DEPARTMENT);
    sendResultServer(senderfd, CALL_TO, ANSWER_EQUAL_DEPARTMENT, resultWrite, clients);
    return;
  }

  /*
   * Comprueba que el cliente con el cual se quiere hablar
   * no este conectado, si es asi, se retorna el control a la
   * funcion que invoco a callto
   */
  if (!connectedClient(destinyDepartmentId, clients)) {
    resultWrite = sendResultClient(senderfd, ANSWER_CLIENT_NOT_CONNECTED);
    sendResultServer(senderfd, CALL_TO, ANSWER_CLIENT_NOT_CONNECTED, resultWrite, clients);
    return;
  }

  int senderDepartmentId = getIdDepartment(clients, senderfd);

  /*
   * Obtiene el descriptor de archivo del socket TCP
   * del cliente receptor de la llamada
   */
  int receiverfd = clients[destinyDepartmentId - 1];

  /*
   * Se crea la notificacion de la llamada entrante para
   * el cliente receptor de dicha llamada
   */
  char notification[BUF_SIZE];
  concatenateTextNotification(notification, CALL_NOTICE_TO_RECEIVER_FIRST_PART, CALL_NOTICE_TO_RECEIVER_SECOND_PART, senderDepartmentId);

  /*
   * El servidor le envia una notificacion de llamada al cliente
   * receptor de dicha llamada
   */
  sendNoticeReceiver(receiverfd, destinyDepartmentId, notification);

  /*
   * El servidor le envia un aviso al cliente emisor de
   * la llamada que contiene el mensaje "call sent", el
   * cual esta contenido en la constante ANSWER_SENDER_CALL_TO
   */
  resultWrite = sendResultClient(senderfd, ANSWER_SENDER_CALL_TO);

  /*
   * Se muestra en la pantalla del servidor, los datos asociados
   * a la ejecucion del comando callto, los cuales son: El nombre
   * del comando, el ID del departamento del cliente que ejecuto
   * el comando y la respuesta del servidor al cliente que ejecuto
   * el comando callto
   */
  sendResultServer(senderfd, CALL_TO, ANSWER_SENDER_CALL_TO, resultWrite, clients);
}

/**
 * Esta funcion tiene la responsabilidad de simular un
 * chat entre dos clientes haciendo uso del protocolo UDP
 *
 * @param senderUdpFd         [descriptor de archivo del socket UDP del cliente emisor]
 * @param destinyDepartmentId [ID del departamento del cliente receptor]
 * @param addrSender          [estructura que contiene la IP y el puerto del cliente emisor]
 * @param addrReceiver        [estructura que contiene la IP y el puerto del cliente receptor]
 * @param clientsUdp          [arreglo que contiene los descriptores de archivo de los sockets UDP usados para interacciones entre los clientes y este servidor]
 */
void sendAudio(int senderUdpFd, int destinyDepartmentId, struct sockaddr_in addrSender, struct sockaddr_in addrReceiver, int clientsUdp[]) {
  int resultOperation;

  /*
   * Validaciones
   * 1. Que el ID del departamento del cliente con el cual
   * se quiere hablar no existe
   *
   * 2. Que el ID del departamento sea igual al ID
   * del departamento del cliente que quiere iniciar
   * la conversacion
   *
   * 3. Que el cliente con el que se quiere hablar
   * no este conectado
   */

  /*
   * Obtiene el ID del departamento del cliente
   * que inicia la conversacion
   */
  int senderDepartmentId = getIdDepartment(clientsUdp, senderUdpFd);

  /*
   * Comprueba que el ID del departamento recibido como
   * argumento no exista, si no existe, se retorna el
   * control a la funcion que invoco a sendAudio
   */
  if (!existDepartment(destinyDepartmentId)) {
    resultOperation = sendNoticeClientUdp(senderUdpFd, ANSWER_NO_DEPARTMENT, addrSender);
    sendReplyServer(senderDepartmentId, S_AUDIO, ANSWER_NO_DEPARTMENT, resultOperation);
    return;
  }

  /*
   * Comprueba que el ID del departamento recibido como
   * argumento sea igual al ID del departamento del cliente
   * que quiere iniciar la conversacion, si es asi, se retorna el
   * control a la funcion que invoco a sendAudio
   */
  if (equalDepartment(senderUdpFd, destinyDepartmentId, clientsUdp)) {
    resultOperation = sendNoticeClientUdp(senderUdpFd, ANSWER_EQUAL_DEPARTMENT, addrSender);
    sendReplyServer(senderDepartmentId, S_AUDIO, ANSWER_EQUAL_DEPARTMENT, resultOperation);
    return;
  }

  /*
   * Comprueba que el cliente con el cual se quiere hablar
   * no este conectado, si es asi, se retorna el control a la
   * funcion que invoco a sendAudio
   */
  if (!connectedClient(destinyDepartmentId, clientsUdp)) {
    resultOperation = sendNoticeClientUdp(senderUdpFd, ANSWER_CLIENT_NOT_CONNECTED, addrSender);
    sendReplyServer(senderDepartmentId, S_AUDIO, ANSWER_CLIENT_NOT_CONNECTED, resultOperation);
    return;
  }

  /*
   * Se le envia el aviso al cliente emisor de que su
   * "mensaje" sendAudio fue enviado al cliente
   * receptor
   */
  sendNoticeClientUdp(senderUdpFd, ANSWER_SENDER_SEND_AUDIO, addrSender);

  /*
   * Obtiene el FD UDP del cliente receptor
   */
  int receiverUdpFd = clientsUdp[destinyDepartmentId - 1];

  /*
   * Se crea la notificacion de la ejecucion del comando
   * sendAudio para el cliente receptor
   */
  char notification[BUF_SIZE];
  concatenateTextNotification(notification, AUDIO_NOTICE_TO_RECEIVER_FIRST_PART, AUDIO_NOTICE_TO_RECEIVER_SECOND_PART, senderDepartmentId);

  /*
   * Se le envia al cliente receptor una notificacion
   * indicandole que alguien le envio un "audio"
   */
  sendNoticeClientUdp(receiverUdpFd, notification, addrReceiver);
}

/**
 * Esta funcion tiene como responsabilidad mostrarle al
 * cliente que ejecuto el comando id, el ID de su departamento
 *
 * @param acceptfd           [descriptor de archivo de un socket TCP]
 * @param clients            [arreglo que contiene los descriptores de archivo de los sockets TCP usados para las conexiones de los clientes con este servidor]
 */
void id(int acceptfd, int clients[]) {
  char destiny[BUF_SIZE] = ANSWER_SENDER_ID;
  char source[BUF_SIZE];

  /*
   * Elimina la basura que haya en este bufer ya que
   * al crearlo no es inicializado, con lo cual es
   * probable que contenga datos basura
   */
  resetCharArray(source);

  int idDepartment = getIdDepartment(clients, acceptfd);

  /*
   * Convierte entero a caracter
   */
  source[0] = idDepartment + '0';

  /*
   * Agrega \n para tener un salto de linea
   */
  source[1] = '\n';

  /*
   * Concatena el valor de la constante
   * ANSWER_SENDER_ID con el ID del departamento
   */
  strcat(destiny, source);

  /*
   * El servidor le envia al cliente que ejecuto el comando
   * id la respuesta "your ID is" seguido del ID
   */
  int resultWrite = sendResultClient(acceptfd, destiny);

  /*
   * Se muestra en la pantalla del servidor, los datos asociados
   * a la ejecucion del comando id, los cuales son: El nombre
   * del comando, el ID del departamento del cliente que ejecuto
   * el comando y la respuesta del servidor al cliente que ejecuto
   * el comando id
   */
  sendResultServer(acceptfd, ID, destiny, resultWrite, clients);
}

/**
 * Esta funcion tiene como responsabilidad indicarle al
 * cliente que el servidor esta en funcionamiento
 *
 * @param acceptfd           [descriptor de archivo de un socket TCP]
 * @param clients            [arreglo que contiene los descriptores de archivo de los sockets TCP usados para las conexiones de los clientes con este servidor]
 */
void ping(int acceptfd, int clients[]) {
  /*
   * El servidor le envia al cliente que ejecuto el comando
   * ping la respuesta "I'm listeinig", la cual esta contenida
   * en la constante ANSWER_SENDER_PING
   */
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_PING);

  /*
   * Se muestra en la pantalla del servidor, los datos asociados
   * a la ejecucion del comando ping, los cuales son: El nombre
   * del comando, el ID del departamento del cliente que ejecuto
   * el comando y la respuesta del servidor al cliente que ejecuto
   * el comando ping
   */
  sendResultServer(acceptfd, PING, ANSWER_SENDER_PING, resultWrite, clients);
}

/**
 * Esta funcion tiene como responsabilidad eliminar el descriptor
 * de archivo del socket TCP del cliente que ejecuto el comando
 * exit, el cual es para la desconexion de parte del cliente
 *
 * @param acceptfd          [descriptor de archivo de un socket TCP]
 * @param disconnection     [variable booleana utilizada para la desconexion de un cliente]
 * @param amountConnections [cantidad de conexiones]
 * @param lock              [variable utilizada para la exclusion mutua de los recursos compartidos por los hilos]
 * @param clients           [arreglo que contiene el descriptor de archivo TCP de cada cliente conectado (arreglo de conexiones)]
 */
void disconnect(int acceptfd, bool* disconnection, int *amountConnections, pthread_mutex_t lock, int clients[], int clientsUdp[]) {
  /*
   * El servidor le envia al cliente que ejecuto el comando
   * exit la respuesta "successful desconnection", la esta
   * contenida en la constante ANSWER_SENDER_EXIT
   */
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_EXIT);

  /*
   * Se muestra en la pantalla del servidor, los datos asociados
   * a la ejecucion del comando exit, los cuales son: El nombre
   * del comando, el ID del departamento del cliente que ejecuto
   * el comando y la respuesta del servidor al cliente que ejecuto
   * el comando exit
   */
  sendResultServer(acceptfd, EXIT, ANSWER_SENDER_EXIT, resultWrite, clients);

  pthread_mutex_lock(&lock);

  /*
   * Se elimina el descriptor de archivo del socket TCP
   * del cliente conectado que ejecuto el comando exit
   */
  removeClient(clients, clientsUdp, acceptfd, amountConnections);
  *disconnection = true;
  pthread_mutex_unlock(&lock);

  printConnections(amountConnections);
}
