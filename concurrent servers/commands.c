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

// TODO: Documentar lo que haga falta documentar

/*
 * Comandos para encender y apagar las luces
 */
void turnon(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {
  /*
   * TODO: Arreglar el codigo muerto que representa la variable resultWrite
   */
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_TURN_ON);
  sendResultServer(acceptfd, TURN_ON, ANSWER_SENDER_TURN_ON, resultWrite, clients);

  pthread_mutex_lock(&lock);
  *sendDefaultMessage = false;
  pthread_mutex_unlock(&lock);
}

void turnoff(int acceptfd,  bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_TURN_OFF);
  sendResultServer(acceptfd, TURN_OFF, ANSWER_SENDER_TURN_OFF, resultWrite, clients);

  pthread_mutex_lock(&lock);
  *sendDefaultMessage = false;
  pthread_mutex_unlock(&lock);
}

/*
 * Comandos para activar y desactivar el riego automatico
 */
void ienable(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_I_ENABLE);
  sendResultServer(acceptfd, I_ENABLE, ANSWER_SENDER_I_ENABLE, resultWrite, clients);

  pthread_mutex_lock(&lock);
  *sendDefaultMessage = false;
  pthread_mutex_unlock(&lock);
}

void idisable(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_I_DISABLE);
  sendResultServer(acceptfd, I_DISABLE, ANSWER_SENDER_I_DISABLE, resultWrite, clients);

  pthread_mutex_lock(&lock);
  *sendDefaultMessage = false;
  pthread_mutex_unlock(&lock);
}

/*
 * Comando para la transmision de un archivo desde el servidor hacia un cliente
 */
void rimage(int acceptFd, bool* sendDefaultMessage, pthread_mutex_t lock, int clients[]) {
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

  pthread_mutex_lock(&lock);
  *sendDefaultMessage = false;
  pthread_mutex_unlock(&lock);
}

/*
 * Comandos para la emision y recepcion de llamadas
 */
void takecall(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {
  int resultWrite = sendResultClient(acceptfd, ANSWER_RECEIVER_TAKE_CALL);
  sendResultServer(acceptfd, TAKE_CALL, ANSWER_RECEIVER_TAKE_CALL, resultWrite, clients);

  pthread_mutex_lock(&lock);
  *sendDefaultMessage = false;
  pthread_mutex_unlock(&lock);
}

void callto(int senderfd, bool* sendDefaultMessage, const char* nameCommandBuf, int destinyDepartmentId, pthread_mutex_t lock, int clients[]) {
  int resultWrite;

  pthread_mutex_lock(&lock);
  *sendDefaultMessage = false;
  pthread_mutex_unlock(&lock);

  // TODO: Documentar
  if (!existDepartment(destinyDepartmentId)) {
    resultWrite = sendResultClient(senderfd, ANSWER_NO_DEPARTMENT);
    sendResultServer(senderfd, CALL_TO, ANSWER_NO_DEPARTMENT, resultWrite, clients);
    return;
  }

  // TODO: Documentar
  if (equalDepartment(senderfd, destinyDepartmentId, clients)) {
    resultWrite = sendResultClient(senderfd, ANSWER_EQUAL_DEPARTMENT);
    sendResultServer(senderfd, CALL_TO, ANSWER_EQUAL_DEPARTMENT, resultWrite, clients);
    return;
  }

  // TODO: Documentar
  if (!connectedClient(destinyDepartmentId, clients)) {
    resultWrite = sendResultClient(senderfd, ANSWER_CLIENT_NOT_CONNECTED);
    sendResultServer(senderfd, CALL_TO, ANSWER_CLIENT_NOT_CONNECTED, resultWrite, clients);
    return;
  }

  // TODO: Modificar
  int senderDepartmentId = getIdDepartment(clients, senderfd);
  int receiverfd = clients[destinyDepartmentId - 1];

  char notification[BUF_SIZE];
  concatenateTextNotification(notification, CALL_NOTICE_TO_RECEIVER_FIRST_PART, CALL_NOTICE_TO_RECEIVER_SECOND_PART, senderDepartmentId);

  // TODO: Documentar
  sendNoticeReceiver(receiverfd, destinyDepartmentId, notification);

  // TODO: Documentar
  resultWrite = sendResultClient(senderfd, ANSWER_SENDER_CALL_TO);
  sendResultServer(senderfd, CALL_TO, ANSWER_SENDER_CALL_TO, resultWrite, clients);
}

/*
 * Comandos para la transmision y recepcion de audio
 */
void sendAudio(int senderUdpFd, bool* sendDefaultMessage, int destinyDepartmentId, pthread_mutex_t lock, struct sockaddr_in addrSender, struct sockaddr_in addrReceiver, int clientsUdp[]) {
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

  char notification[BUF_SIZE];
  concatenateTextNotification(notification, AUDIO_NOTICE_TO_RECEIVER_FIRST_PART, AUDIO_NOTICE_TO_RECEIVER_SECOND_PART, senderDepartmentId);

  /*
   * Se le envia al cliente receptor una notificacion
   * indicandole que alguien le envio un "audio"
   */
  sendNoticeClientUdp(receiverUdpFd, notification, addrReceiver);

  pthread_mutex_lock(&lock);
  *sendDefaultMessage = false;
  pthread_mutex_unlock(&lock);
}

void recaudio(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {
  int resultWrite = sendResultClient(acceptfd, ANSWER_RECEIVER_REC_AUDIO);
  sendResultServer(acceptfd, R_AUDIO, ANSWER_RECEIVER_REC_AUDIO, resultWrite, clients);

  pthread_mutex_lock(&lock);
  *sendDefaultMessage = false;
  pthread_mutex_unlock(&lock);
}

/*
 * Otros comandos
 */
void id(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {
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
   * Concatena el contenido de ambos arreglos
   * de char y el resultado lo coloca en el primer
   * arreglo de char que se pasa como argumento
   *
   * Concatena el valor de la constante
   * ANSWER_SENDER_ID con el ID del departamento
   */
  strcat(destiny, source);

  int resultWrite = sendResultClient(acceptfd, destiny);
  sendResultServer(acceptfd, ID, destiny, resultWrite, clients);

  pthread_mutex_lock(&lock);
  *sendDefaultMessage = false;
  pthread_mutex_unlock(&lock);
}

void ping(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_PING);
  sendResultServer(acceptfd, PING, ANSWER_SENDER_PING, resultWrite, clients);

  pthread_mutex_lock(&lock);
  *sendDefaultMessage = false;
  pthread_mutex_unlock(&lock);
}

void disconnect(int acceptfd,  bool* sendDefaultMessage, bool* disconnection, const char* buf, int *amountConnections, pthread_mutex_t lock, int clients[]) {
  int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_EXIT);
  sendResultServer(acceptfd, EXIT, ANSWER_SENDER_EXIT, resultWrite, clients);

  pthread_mutex_lock(&lock);
  removeClient(clients, acceptfd, amountConnections);
  *sendDefaultMessage = false;
  *disconnection = true;
  pthread_mutex_unlock(&lock);

  printConnections(amountConnections);
}
