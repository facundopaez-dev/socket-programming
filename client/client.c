#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include "headers/utildefinitions.h"
#include "../concurrent servers/headers/answers.h"
#include "../concurrent servers/headers/namecommands.h"
#include "../concurrent servers/headers/confirmations.h"
#include "../concurrent servers/headers/modes.h"
#include "../concurrent servers/headers/notices.h"
#define IP "127.0.0.1"
#define PORT "50001"

pthread_mutex_t lock;

struct structparams {
  int socketTcpFd;
  int socketUdpFd;
};

void *sendRequest(void *args);

void *receiveTcpResponse(void *args);

void *receiveUdpResponse(void *args);

int main(int argc, char *argv[]) {
  int socketTcpFd = getFdSocketTcp(IP, PORT);
  int socketUdpFd = getFdSocketUdp(IP, PORT);

  int resultSendThread;
  int resultReceiveTcpThread;
  int resultReceiveUdpThread;

  pthread_t sendRequestThread;
  pthread_t receiveResponseTcpThread;
  pthread_t receiveResponseUdpThread;

  struct structparams params;
  params.socketTcpFd = socketTcpFd;
  params.socketUdpFd = socketUdpFd;

  resultSendThread = pthread_create(&sendRequestThread, NULL, sendRequest, (void *) &params);
  resultReceiveTcpThread = pthread_create(&receiveResponseTcpThread, NULL, receiveTcpResponse, (void *) &params);
  resultReceiveUdpThread = pthread_create(&receiveResponseUdpThread, NULL, receiveUdpResponse, (void *) &params);

  if (resultSendThread != 0) {
    printf("ERROR: return code from pthread_create() is %i\n", resultSendThread);
    close(socketTcpFd);
    close(socketUdpFd);
    exit(EXIT_FAILURE);
  }

  if (resultReceiveTcpThread != 0) {
    printf("ERROR: return code from pthread_create() is %i\n", resultReceiveTcpThread);
    close(socketTcpFd);
    close(socketUdpFd);
    exit(EXIT_FAILURE);
  }

  if (resultReceiveUdpThread != 0) {
    printf("ERROR: return code from pthread_create() is %i\n", resultReceiveUdpThread);
    close(socketTcpFd);
    close(socketUdpFd);
    exit(EXIT_FAILURE);
  }

  /*
   * Esto evita que el programa termine su ejecucion
   *
   * Esta instruccion hace que el hilo principal, espere
   * la terminacion del hilo que se le pasa como primer
   * argumento, para fianlizar su ejecucion
   */
  pthread_join(sendRequestThread, NULL);
} // End main

void *sendRequest(void *args) {
  struct structparams *params = (struct structparams *) args;

  // struct sockaddr_storage claddr;
  // socklen_t len;

  int numRead;
  int numWrite;
  int idDepartment;
  int socketTcpFd = params -> socketTcpFd;
  int socketUdpFd = params -> socketUdpFd;

  bool sendInvalidCommand = true;

  char inputBuffer[BUF_SIZE];
  char nameCommandBuf[BUF_SIZE];
  char sendBuffer[BUF_SIZE] = "";

  for(;;) {
    numRead = read(0, inputBuffer, BUF_SIZE);

    if (numRead == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    /*
     * Esto suprime el \n de la cadena enviada por el cliente
     * haciendo que las cadenas de texto que son iguales no
     * den como resultado (por la funcion strcmp) que son
     * diferentes
     */
    inputBuffer[numRead - 1] = '\0';

    /*
     * Separa el nombre del comando del argumento, el cual
     * es el ID de un departamento
     *
     * Hay que recordar que el ID de un departamento en el
     * arreglo de conexiones es una celda del arreglo, y
     * por ende, es el indice de esa celda
     */
    sscanf(inputBuffer, "%s%d", nameCommandBuf, &idDepartment);

    /*
     * Es necesario reiniciar el buffer de envio, en caso
     * contrario el programa cliente mostrata por pantalla
     * resultados erroneos
     */
    resetBuffer(sendBuffer);

    /*
     * Si el comando es para TCP, se tiene que ejecutar
     * haciendo uso del protocolo TCP
     *
     * Comandos: Luces, riego, imagen, desconexion,
     * ping y id
     */
    if (strcmp(nameCommandBuf, TURN_ON) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(TURN_ON, sendBuffer);

      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, TURN_OFF) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(TURN_OFF, sendBuffer);

      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, I_ENABLE) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(I_ENABLE, sendBuffer);

      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, I_DISABLE) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(I_DISABLE, sendBuffer);

      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, S_IMAGE) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(S_IMAGE, sendBuffer);

      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, R_IMAGE) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(R_IMAGE, sendBuffer);

      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, EXIT) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(EXIT, sendBuffer);

      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
      exit(EXIT_SUCCESS);
    }

    if(strcmp(nameCommandBuf, PING) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(PING, sendBuffer);

      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, ID) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(ID, sendBuffer);

      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, CALL_TO) == 0 ) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(CALL_TO, sendBuffer);

      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, TAKE_CALL) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(TAKE_CALL, sendBuffer);

      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    /*
     * Si el comando es para UDP, se tiene que ejecutar
     * haciendo uso del protocolo UDP
     *
     * Comandos: Audio
     */
    if (strcmp(nameCommandBuf, S_AUDIO) == 0) {
      int numRead;

      strcat(sendBuffer, FIELD_UDP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(S_AUDIO, sendBuffer);

      /*
       * El cliente le envia el comando sendaudio al servidor
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);

      socklen_t len;
      struct sockaddr_in addr;
      void *buffer;

      /*
       * El cliente recibe los datos del servidor asociados a su
       * socket UDP
       */
      numRead = read(socketTcpFd, (void *) &addr, sizeof(addr));
      // addr = (struct sockaddr) buffer;

      printf("%s\n", "");
      printf("%s\n", "[CLIENT] UDP data");
      printf("[CLIENT] Port: %d\n", ntohs(addr.sin_port));
      printf("[CLIENT] IP adress: %s\n", inet_ntoa(addr.sin_addr));

      // struct sockaddr_storage claddr;

      // Recibir del servidor sus datos
      // completar estructura addr con los datos del servidor
      // luego usarla con sendto

      // int resultPeerName = getpeername(socketUdpFd, &addr, &len);
      //
      // if (resultPeerName == -1) {
      //   perror("getpeername");
      //   exit(EXIT_FAILURE);
      // }

      char buf[BUF_SIZE];

      /* Recibe datagramas y retorna copias a los emisores */
      for (;;) {
        numRead = read(0, buf, BUF_SIZE);

        len = sizeof(struct sockaddr);
        // len = sizeof(struct sockaddr_storage);
        // printf("%s\n", "ENTRO");
        // numRead = recvfrom(socketUdpFd, buf, BUF_SIZE, 0, (struct sockaddr *) &claddr, &len);
        // printf("%s\n", buf);

        if (numRead == -1) {
          fprintf(stderr, "%s\n", buf);
        }

        // if (sendto(socketUdpFd, buf, numRead, 0, (struct sockaddr *) &claddr, len) != numRead) {
        if (sendto(socketUdpFd, buf, numRead, 0, (struct sockaddr *) &addr, len) != numRead) {
          fprintf(stderr, "%s\n", buf);
        }

      }

      // len = sizeof(struct sockaddr_storage);
      // sendto(socketUdpFd, inputBuffer, numRead, 0, (struct sockaddr *) &claddr, len);
      // write(socketTcpFd, TCP_MODE, strlen(TCP_MODE) + 1);
      sendInvalidCommand = false;
    }

    // NOTE: Puede que este comando sea borrado
    // if (strcmp(nameCommandBuf, R_AUDIO) == 0) {
    //   numWrite = write(socketTcpFd, nameCommandBuf, strlen(nameCommandBuf) + 1);
    //   sendInvalidCommand = false;
    // }

    /*
     * Si la variable booleana sendInvalidCommand es verdadera
     * el cliente le tiene que avisar al servidor que se ingreso
     * un comando invalido
     */
    if (sendInvalidCommand) {
      /*
       * El cliente le envia al servidor el aviso de que se
       * ingreso un comando invalido
       */
      write(socketTcpFd, INVALID_COMMAND_NOTICE, strlen(INVALID_COMMAND_NOTICE) + 1);
      numWrite = write(socketTcpFd, inputBuffer, strlen(inputBuffer) + 1);
    }

    /*
     * Si sendInvalidCommand tiene el valor falso
     * tiene que ser reiniciada, porque en caso de no
     * hacerlo el programa cliente va a funcionar
     * de forma erronea
     */
    if (!sendInvalidCommand) {
      sendInvalidCommand = true;
    }

    if (numWrite == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    }

  } // End for

}

void *receiveTcpResponse(void *args) {
  struct structparams *params = (struct structparams *) args;

  int numRead;
  int socketTcpFd = params -> socketTcpFd;

  char outputBuffer[BUF_SIZE];

  for(;;) {
    numRead = read(socketTcpFd, outputBuffer, BUF_SIZE);

    if (numRead == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    printf("[CLIENT] Response from server: %s\n", outputBuffer);

    /*
     * Si se llega al limite de conexiones del servidor, este
     * envia, al cliente que se quiere conectar, el mensaje
     * "Connection limit reached, try again later", con lo cual
     * se tiene que comprobar que el arreglo de caracteres outputBuffer
     * contiene este mensaje para evitar que dicho cliente se quede
     * en un bucle infinito escribiendo por pantalla el contenido
     * de outputBuffer
     */
    if (strcmp(outputBuffer, ANSWER_LIMIT_CONNECTIONS) == 0) {
      exit(EXIT_SUCCESS);
    }

  } // End for

}

void *receiveUdpResponse(void *args) {
  struct structparams *params = (struct structparams *) args;

  int socketUdpFd = params -> socketUdpFd;
  int numRead;

  char outputBuffer[BUF_SIZE];

  socklen_t len;
  struct sockaddr_storage claddr;

  for(;;) {
    len = sizeof(struct sockaddr_storage);
    numRead = recvfrom(socketUdpFd, outputBuffer, BUF_SIZE, 0, (struct sockaddr *) &claddr, &len);

    if (numRead == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    printf("[CLIENT] Response from server: %s\n", outputBuffer);

    /*
     * Si se llega al limite de conexiones del servidor, este
     * envia, al cliente que se quiere conectar, el mensaje
     * "Connection limit reached, try again later", con lo cual
     * se tiene que comprobar que el arreglo de caracteres outputBuffer
     * contiene este mensaje para evitar que dicho cliente se quede
     * en un bucle infinito escribiendo por pantalla el contenido
     * de outputBuffer
     */
    // if (strcmp(outputBuffer, ANSWER_LIMIT_CONNECTIONS) == 0) {
    //   exit(EXIT_SUCCESS);
    // }

  } // End for

}
