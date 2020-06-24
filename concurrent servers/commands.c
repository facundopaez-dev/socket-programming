#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "answers.h"
#include "namecommands.h"
#include "utildefinitions.h"
#include "commandsdefinitions.h"
#include "confirmations.h"
#include "notices.h"

// TODO: Documentar lo que haga falta documentar

/*
 * Comandos para encender y apagar las luces
 */
void turnon(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {

  if (strcmp(buf, TURN_ON) == 0) {
    int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_TURN_ON);
    sendResultServer(acceptfd, TURN_ON, ANSWER_SENDER_TURN_ON, resultWrite, clients);
    pthread_mutex_lock(&lock);
    *sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

void turnoff(int acceptfd,  bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {

  if (strcmp(buf, TURN_OFF) == 0) {
    int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_TURN_OFF);
    sendResultServer(acceptfd, TURN_OFF, ANSWER_SENDER_TURN_OFF, resultWrite, clients);
    pthread_mutex_lock(&lock);
    *sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

/*
 * Comandos para activar y desactivar el riego automatico
 */
void ienable(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {

  if (strcmp(buf, I_ENABLE) == 0) {
    int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_I_ENABLE);
    sendResultServer(acceptfd, I_ENABLE, ANSWER_SENDER_I_ENABLE, resultWrite, clients);
    pthread_mutex_lock(&lock);
    *sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

void idisable(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {

  if (strcmp(buf, I_DISABLE) == 0) {
    int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_I_DISABLE);
    sendResultServer(acceptfd, I_DISABLE, ANSWER_SENDER_I_DISABLE, resultWrite, clients);
    pthread_mutex_lock(&lock);
    *sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

/*
 * Comandos para la transmision y recepcion de imagenes
 */
void simage(int senderfd, bool* sendDefaultMessage, const char* nameCommandBuf, int destinyDepartmentId, pthread_mutex_t lock, int clients[]) {

  if (strcmp(nameCommandBuf, S_IMAGE) == 0) {
    int resultWrite;
    int numRead;

    FILE* newFile = NULL;
    char buf[BUF_SIZE];

    pthread_mutex_lock(&lock);
    *sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);

    /*
     * Comprueba si el departamento solicitado
     * existe, en caso de que no exista se le avisa
     * de esto al cliente emisor de la imagen
     */
    if (!existDepartment(destinyDepartmentId)) {
      resultWrite = sendResultClient(senderfd, ANSWER_NO_DEPARTMENT);
      sendResultServer(senderfd, S_IMAGE, ANSWER_NO_DEPARTMENT, resultWrite, clients);
      return;
    }

    // TODO: Modificar comentario
    /*
     * Si el departamento al que se quiere enviar algo
     * es igual al departamento del emisor, no se tiene
     * que enviar nada y se tiene que avisar de esto
     * al emisor
     */
    if (equalDepartment(senderfd, destinyDepartmentId, clients)) {
      resultWrite = sendResultClient(senderfd, ANSWER_EQUAL_DEPARTMENT);
      sendResultServer(senderfd, S_IMAGE, ANSWER_EQUAL_DEPARTMENT, resultWrite, clients);
      return;
    }

    /*
     * Si el departamento existe, se comprueba que
     * el cliente de dicho departamento este conectado,
     * en caso de que el cliente receptor no este conectado
     * se le avisa al cliente emisor de esto
     */
    if (!connectedClient(destinyDepartmentId, clients)) {
      resultWrite = sendResultClient(senderfd, ANSWER_CLIENT_NOT_CONNECTED);
      sendResultServer(senderfd, S_IMAGE, ANSWER_CLIENT_NOT_CONNECTED, resultWrite, clients);
      return;
    }

    /*
     * Se muestra en el servidor el cliente que ejecuto el comando simage
     */
    sendResultServer(senderfd, S_IMAGE, ANSWER_SENDER_S_IMAGE, resultWrite, clients);

    /*
     * Se crea un archivo para copiar el contenido del archivo que
     * un cliente quiere enviar a otro cliente
     */
    // newFile = fopen("newFile.txt", "w");

    // printf("%s\n", "ANTES DE ENVIAR EL AVISO");

    /*
     * Se le avisa al cliente receptor que hay un cliente emisor
     * que le quiere enviar un archivo
     */
    int receiverfd = clients[destinyDepartmentId - 1];
    sendNoticeReceiver(receiverfd, destinyDepartmentId, FILE_SEND_NOTIFICATION);

    /*
     * Se confirma al cliente emisor que el cliente receptor esta disponible para
     * la transmision del archivo usado en el comando simage
     */
    resultWrite = write(senderfd, AVAILABLE_CUSTOMER_CONFIRMATION, strlen(AVAILABLE_CUSTOMER_CONFIRMATION) + 1);

    if (resultWrite == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    }

    // while ((numRead = read(senderfd, buf, BUF_SIZE)) > 0) {
    //
    //   if (strcmp(buf, EOF_CONFIRMATION) != 0) {
    //     fputs(buf, newFile);
    //   }
    //
    //   if (strcmp(buf, EOF_CONFIRMATION) == 0) {
    //     break;
    //   }
    //
    // }

    /*
     * Se cierra el archivo previamente abierto
     */
    // fclose(newFile);
    // printf("%s\n", "DESPUES DEL WHILE");

    /*
     * Se le avisa al cliente receptor que alguien le quiere enviar un archivo
     */
    // int receiverfd = clients[destinyDepartmentId - 1];
    // resultWrite = write(receiverfd, FILE_SEND_NOTIFICATION, strlen(FILE_SEND_NOTIFICATION) + 1);
    //
    // if (resultWrite == -1) {
    //   perror("write");
    //   exit(EXIT_FAILURE);
    // }

    // printf("%s\n", "ENTRO");

    /*
     * Si el flujo de ejecucion del programa llega hasta aca
     * es porque ya esta todo listo para que el cliente emisor
     * envie al servidor el archivo, y su vez que este ultimo
     * le envie al cliente receptor el archivo
     */
    while ((numRead = read(senderfd, buf, BUF_SIZE)) > 0) {
      printf("%s\n", buf);

      resultWrite = write(receiverfd, buf, strlen(buf) + 1);

      if (resultWrite == -1) {
        perror("write");
        exit(EXIT_FAILURE);
      }


      if (strcmp(buf, EOF_CONFIRMATION) == 0) {
        resultWrite = write(receiverfd, EOF_CONFIRMATION, strlen(EOF_CONFIRMATION) + 1);

        if (resultWrite == -1) {
          perror("write");
          exit(EXIT_FAILURE);
        }

        break;
      }

    } // End while

    printf("%s\n", "SERVIDOR TERMINO LA TRANSFERENCIA DEL ARCHIVO");

    // TODO: Modificar
    // int senderDepartmentId = getIdDepartment(clients, senderfd);
    // int receiverfd = clients[destinyDepartmentId - 1];

    // char notification[BUF_SIZE];
    // concatenateTextNotification(notification, IMAGE_NOTICE_TO_RECEIVER_FIRST_PART, IMAGE_NOTICE_TO_RECEIVER_SECOND_PART, senderDepartmentId);

    /*
     * Si el flujo de ejecucion llega hasta aca, es porque el
     * cliente receptor (al que el cliente emisor le quiere
     * enviar una imagen), esta conectado, en este caso, al
     * cliente receptor se le tiene que enviar un aviso de
     * que alguien le envio una imagen
     */
    // sendNoticeReceiver(receiverfd, destinyDepartmentId, notification);

    // TODO: Documentar
    // resultWrite = sendResultClient(senderfd, ANSWER_SENDER_S_IMAGE);
    // sendResultServer(senderfd, S_IMAGE, ANSWER_SENDER_S_IMAGE, resultWrite, clients);
  }

}

void rimage(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {

  if (strcmp(buf, R_IMAGE) == 0) {
    int resultWrite = sendResultClient(acceptfd, ANSWER_RECEIVER_R_IMAGE);
    sendResultServer(acceptfd, R_IMAGE, ANSWER_RECEIVER_R_IMAGE, resultWrite, clients);
    pthread_mutex_lock(&lock);
    *sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

/*
 * Comandos para la emision y recepcion de llamadas
 */
void takecall(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {

  if (strcmp(buf, TAKE_CALL) == 0) {
    int resultWrite = sendResultClient(acceptfd, ANSWER_RECEIVER_TAKE_CALL);
    sendResultServer(acceptfd, TAKE_CALL, ANSWER_RECEIVER_TAKE_CALL, resultWrite, clients);
    pthread_mutex_lock(&lock);
    *sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

void callto(int senderfd, bool* sendDefaultMessage, const char* nameCommandBuf, int destinyDepartmentId, pthread_mutex_t lock, int clients[]) {

  if (strcmp(nameCommandBuf, CALL_TO) == 0) {
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

}

/*
 * Comandos para la transmision y recepcion de audio
 */
void sendaudio(int senderfd, bool* sendDefaultMessage, const char* nameCommandBuf, int destinyDepartmentId, pthread_mutex_t lock, int clients[]) {

  if (strcmp(nameCommandBuf, S_AUDIO) == 0) {
    int resultWrite;

    pthread_mutex_lock(&lock);
    *sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);

    // TODO: Documentar
    if (!existDepartment(destinyDepartmentId)) {
      resultWrite = sendResultClient(senderfd, ANSWER_NO_DEPARTMENT);
      sendResultServer(senderfd, S_AUDIO, ANSWER_NO_DEPARTMENT, resultWrite, clients);
      return;
    }

    // TODO: Documentar
    if (equalDepartment(senderfd, destinyDepartmentId, clients)) {
      resultWrite = sendResultClient(senderfd, ANSWER_EQUAL_DEPARTMENT);
      sendResultServer(senderfd, S_AUDIO, ANSWER_EQUAL_DEPARTMENT, resultWrite, clients);
      return;
    }

    // TODO: Documentar
    if (!connectedClient(destinyDepartmentId, clients)) {
      resultWrite = sendResultClient(senderfd, ANSWER_CLIENT_NOT_CONNECTED);
      sendResultServer(senderfd, S_AUDIO, ANSWER_CLIENT_NOT_CONNECTED, resultWrite, clients);
      return;
    }

    // TODO: Modificar
    int senderDepartmentId = getIdDepartment(clients, senderfd);
    int receiverfd = clients[destinyDepartmentId - 1];

    char notification[BUF_SIZE];
    concatenateTextNotification(notification, AUDIO_NOTICE_TO_RECEIVER_FIRST_PART, AUDIO_NOTICE_TO_RECEIVER_SECOND_PART, senderDepartmentId);

    // TODO: Documentar
    sendNoticeReceiver(receiverfd, destinyDepartmentId, notification);

    // TODO: Documentar
    resultWrite = sendResultClient(senderfd, ANSWER_SENDER_SEND_AUDIO);
    sendResultServer(senderfd, S_AUDIO, ANSWER_SENDER_SEND_AUDIO, resultWrite, clients);
  }

}

void recaudio(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {

  if (strcmp(buf, R_AUDIO) == 0) {
    int resultWrite = sendResultClient(acceptfd, ANSWER_RECEIVER_REC_AUDIO);
    sendResultServer(acceptfd, R_AUDIO, ANSWER_RECEIVER_REC_AUDIO, resultWrite, clients);
    pthread_mutex_lock(&lock);
    *sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

/*
 * Otros comandos
 */
 void id(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {

   if (strcmp(buf, ID) == 0) {
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
      */
     strcat(destiny, source);

     int resultWrite = sendResultClient(acceptfd, destiny);
     sendResultServer(acceptfd, ID, destiny, resultWrite, clients);
     pthread_mutex_lock(&lock);
     *sendDefaultMessage = false;
     pthread_mutex_unlock(&lock);
   }

 }

void ping(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]) {

  if (strcmp(buf, PING) == 0) {
    int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_PING);
    sendResultServer(acceptfd, PING, ANSWER_SENDER_PING, resultWrite, clients);
    pthread_mutex_lock(&lock);
    *sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

void disconnect(int acceptfd,  bool* sendDefaultMessage, bool* disconnection, const char* buf, int *amountConnections, pthread_mutex_t lock, int clients[]) {

  if (strcmp(buf, EXIT) == 0) {
    int resultWrite = sendResultClient(acceptfd, ANSWER_SENDER_EXIT);
    sendResultServer(acceptfd, EXIT, ANSWER_SENDER_EXIT, resultWrite, clients);

    pthread_mutex_lock(&lock);
    removeClient(clients, acceptfd, amountConnections);
    *sendDefaultMessage = false;
    *disconnection = true;
    pthread_mutex_unlock(&lock);

    close(acceptfd);
    printConnections(amountConnections);
  }

}
