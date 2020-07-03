#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

// Headers
#include "../concurrent servers/headers/answers.h"
#include "../concurrent servers/headers/confirmations.h"
#include "../concurrent servers/headers/modes.h"
#include "../concurrent servers/headers/namecommands.h"
#include "../concurrent servers/headers/notices.h"
#include "headers/utildefinitions.h"

// Constants
#define IP "127.0.0.1"
#define PORT "50001"
#define FILE_NAME "Pinwheel Galaxy Messier 101.jpg"

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

      displayCommandExecuted(TURN_ON);
      displayDataSent(sendBuffer);

      /*
       * El cliente le envia el comando turnon al servidor con
       * el que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, TURN_OFF) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayCommandExecuted(TURN_OFF);
      displayDataSent(sendBuffer);

      /*
       * El cliente le envia el comando turnoff al servidor con
       * el que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, I_ENABLE) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayCommandExecuted(I_ENABLE);
      displayDataSent(sendBuffer);

      /*
       * El cliente le envia el comando ienable al servidor con
       * el que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, I_DISABLE) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayCommandExecuted(I_DISABLE);
      displayDataSent(sendBuffer);

      /*
       * El cliente le envia el comando idisable al servidor con
       * el que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, R_IMAGE) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayCommandExecuted(R_IMAGE);
      displayDataSent(sendBuffer);

      /*
       * Pasos para recibir un archivo de parte del servidor
       * 1. Solicitar el archivo mediante el comando rimage
       * 2. Recibir la confirmacion o no de un archivo
       * existente en el directorio del servidor
       * 3. Recibir tamaño del archivo
       * 4. Leer cada byte del archivo enviado por el servidor
       *
       * Los pasos 2, 3 y 4 estan comentados en la funcion
       * receiveTcpResponse()
       */

      /*
       * 1. El cliente le envia el comando rimage al servidor con el
       * que esta conectado, de esta forma le solicita al servidor
       * un archivo
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);

      if (numWrite == -1) {
        perror("write");
        exit(EXIT_FAILURE);
      }

      sendInvalidCommand = false;
    } // End if rimage

    if(strcmp(nameCommandBuf, EXIT) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayCommandExecuted(EXIT);
      displayDataSent(sendBuffer);

      /*
       * El cliente le envia el comando exit al servidor con
       * el que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
      exit(EXIT_SUCCESS);
    }

    if(strcmp(nameCommandBuf, PING) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayCommandExecuted(PING);
      displayDataSent(sendBuffer);

      /*
       * El cliente le envia el comando ping al servidor con
       * el que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, ID) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayCommandExecuted(ID);
      displayDataSent(sendBuffer);

      /*
       * El cliente le envia el comando id al servidor con
       * el que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, CALL_TO) == 0 ) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayCommandExecuted(CALL_TO);
      displayDataSent(sendBuffer);

      /*
       * El cliente le envia el comando callto al servidor con
       * el que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, TAKE_CALL) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayCommandExecuted(TAKE_CALL);
      displayDataSent(sendBuffer);

      /*
       * El cliente le envia el comando takecall al servidor con
       * el que esta conectado
       */
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
      strcat(sendBuffer, FIELD_UDP);
      strcat(sendBuffer, inputBuffer);

      displayCommandExecuted(S_AUDIO);
      displayDataSent(sendBuffer);

      /*
       * El cliente le envia el comando sendaudio al servidor con el
       * que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);

      /*
       * Pasos para conectar el socket UDP del cliente con
       * el socket UDP del servidor
       *
       * 1. Recibir los datos del servidor, estos son su
       * puerto y su direccion IP
       * 2. Cargar una estructura sockaddr_in con los datos
       * del servidor
       * 3. Luego, utilizar la estructura de tipo sockaddr_in
       * en la llamada al sistema sendto
       */
      socklen_t len;
      struct sockaddr_in addr;

      /*
       * El cliente recibe los datos del servidor asociados al socket
       * UDP del mismo, los cuales son la direccion IP y el puerto
       */
      numRead = read(socketTcpFd, (void *) &addr, sizeof(addr));

      printf("%s\n", "");
      printf("%s\n", "[CLIENT] UDP data");
      printf("[CLIENT] Port: %d\n", ntohs(addr.sin_port));
      printf("[CLIENT] IP adress: %s\n", inet_ntoa(addr.sin_addr));

      /*
       * Recibe lo que se ingresa por teclado y lo envia al
       * servidor con el que esta conectado
       */
      for (;;) {
        numRead = read(0, sendBuffer, BUF_SIZE);

        len = sizeof(addr);

        if (numRead == -1) {
          fprintf(stderr, "%s\n", sendBuffer);
        }

        if (sendto(socketUdpFd, sendBuffer, numRead, 0, (struct sockaddr *) &addr, len) != numRead) {
          fprintf(stderr, "%s\n", sendBuffer);
        }

      }

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
      write(socketTcpFd, INVALID_COMMAND_NOTICE, BUF_SIZE);

      /*
       * El cliente le envia al servidor lo que ingresado
       * por el teclado es un comando invalido
       */
      numWrite = write(socketTcpFd, inputBuffer, BUF_SIZE);
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
  int numWrite;
  int socketTcpFd = params -> socketTcpFd;
  int resultRecv;

  char outputBuffer[BUF_SIZE];

  for(;;) {
    numRead = read(socketTcpFd, outputBuffer, BUF_SIZE);

    if (numRead == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    if (strcmp(outputBuffer, S_IMAGE) == 0) {
      /*
       * 2. El cliente, en esta linea de codigo fuente, recibe la
       * confirmacion de parte del servidor de si tiene o no
       * un archivo para la transferencia
       */
      numRead = read(socketTcpFd, outputBuffer, BUF_SIZE);

      if (numRead == -1) {
        perror("read");
        exit(EXIT_FAILURE);
      }

      /*
       * El cliente comprueba si lo que recibio de parte
       * del servidor es la confirmacion positiva de la
       * existencia de un archivo
       */
      if (strcmp(outputBuffer, EXISTING_FILE) == 0) {
        off_t fileSize;
        char buffer[BUF_SIZE];

        int bytesReceived = 0;
        int flags = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
        int fileFd = open(FILE_NAME, O_CREAT | O_TRUNC | O_WRONLY, flags);

        /*
         * 3. El cliente recibe del servidor el tamaño del archivo
         * que va a recibir
         */
        resultRecv = recv(socketTcpFd, &fileSize, sizeof(fileSize), 0);

        if (resultRecv == -1) {
          perror("recv");
          exit(EXIT_FAILURE);
        }

        fileSize = ntohl(fileSize);
        printf("[CLIENT][TCP] File size received: %ld\n", fileSize);

        /*
         * 4. El cliente recibe de parte del servidor los
         * bytes del archivo, en este parte del codigo fuente
         * es en donde inicia la transferencia del archivo
         * desde al servidor a este cliente
         */
        while (bytesReceived < fileSize) {
          resultRecv = recv(socketTcpFd, buffer, BUF_SIZE, 0);

          if (resultRecv == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
          }

          write(fileFd, buffer, resultRecv);
          bytesReceived += resultRecv;

          if (bytesReceived == fileSize) {
            break;
          }

        }

        printf("[CLIENT][TCP] File transfer completed\n", "");
        close(fileFd);
      } // Enf if EXISTING_FILE

      /*
       * El cliente comprueba si lo que recibio de parte
       * del servidor es la confirmacion negativa de la
       * existencia de un archivo
       */
      if (strcmp(outputBuffer, NONEXISTING_FILE) == 0) {
        printf("[CLIENT] The server doesn't have a file\n", "");
      }

    } // End if S_IMAGE

    if (strcmp(outputBuffer, S_AUDIO) == 0) {
      printf("[CLIENT][RECEIVER][TCP] ENTRO EN IF S_AUDIO\n", "");
      struct sockaddr_in addr;
      socklen_t len = sizeof(addr);

      int resultGetSockName = getsockname(socketTcpFd, (struct sockaddr *) &addr, &len);

      if (resultGetSockName == -1) {
        perror("getsockname");
        exit(EXIT_FAILURE);
      }

      /*
       * El cliente receptor le envia al servidor (mediante el socket TCP)
       * su direccion IP y puerto
       */
      numWrite = write(socketTcpFd, (const void *) &addr, sizeof(addr));

      if (numWrite == -1) {
        perror("write");
        exit(EXIT_FAILURE);
      }

      printf("[CLIENT][RECEIVER][TCP] Mis datos son\n", "");
      printf("[CLIENT][RECEIVER][TCP] Port: %d\n", ntohs(addr.sin_port));
      printf("[CLIENT][RECEIVER][TCP] IP adress: %s\n", inet_ntoa(addr.sin_addr));
      printf("[CLIENT][RECEIVER][TCP] IP and port sended\n", "");
    } // End if S_AUDIO

    printf("[CLIENT][TCP] Response from server: %s\n", outputBuffer);

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
  struct sockaddr_in addr;
  len = sizeof(addr);

  for(;;) {
    // len = sizeof(addr);
    numRead = recvfrom(socketUdpFd, outputBuffer, BUF_SIZE, 0, (struct sockaddr *) &addr, &len);
    // outputBuffer[numRead - 1] = '\0';

    if (numRead == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    printf("[CLIENT][RECEIVER][UDP] %s\n", "IF Notification received");
    if (strcmp(outputBuffer, S_AUDIO) == 0) {
      printf("%s\n", "[CLIENT][RECEIVER][UDP] Notification received");
    }

    printf("[CLIENT][RECEIVER][UDP] Response from server: %s\n", outputBuffer);

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
