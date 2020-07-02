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
#include "headers/utildefinitions.h"
#include "../concurrent servers/headers/answers.h"
#include "../concurrent servers/headers/namecommands.h"
#include "../concurrent servers/headers/modes.h"
#include "../concurrent servers/headers/notices.h"

// Constants
#define FILE_NAME "M101_hires_STScI-PRC2006-10a.jpg"
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

      displayDataSent(TURN_OFF, sendBuffer);

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

      displayDataSent(I_ENABLE, sendBuffer);

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

      displayDataSent(I_DISABLE, sendBuffer);

      /*
       * El cliente le envia el comando idisable al servidor con
       * el que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, S_IMAGE) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(S_IMAGE, sendBuffer);

      /*
       * El cliente le envia el comando simage al servidor con
       * el que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);

      /*
       * Pasos para transferir un archivo al servidor
       * 1. Abrir el archivo a enviar
       * 2. Determinar su tamaño
       * 3. Enviar al servidor el tamaño del archivo abierto
       * 2. Enviar el archivo
       */
      int fileFd = open(FILE_NAME, O_RDONLY);

      if (fileFd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
      } else {
        printf("[CLIENT] %s\n", "Open image file");
      }

      off_t fileSize;
      struct stat fileInfo;

      /*
       * Obtiene la informacion del archivo asociado
       * al descriptor de archivo que se pasa como
       * primer argumento, y coloca dicha informacion
       * en la estructura fileInfo
       */
      fstat(fileFd, &fileInfo);
      fileSize = htonl(fileInfo.st_size);

      /*
       * El cliente le envia al servidor, con el cual esta
       * conectado, el tamaño del archivo abierto
       */
      int resultSend = send(socketTcpFd, (void *) &fileSize, sizeof(fileSize), 0);

      if (resultSend == -1) {
        perror("send");
        exit(EXIT_FAILURE);
      }

      int bytesSended = 1;
      int resultSendFile;

      printf("[CLIENT SENDER] st_size: %ld\n", fileInfo.st_size);

      /*
       * El cliente le envia el archivo, previamente
       * abierto, al servidor con el que esta conectado
       */
      while (bytesSended > 0) {
        resultSendFile = sendfile(socketTcpFd, fileFd, NULL, BUF_SIZE);

        if (resultSendFile == -1) {
          perror("sendfile");
          exit(EXIT_FAILURE);
        }

        printf("Bytes sended: %d\n", bytesSended);
        // printf("File size: %ld\n", fileSize);
        printf("resultSendFile: %d\n", resultSendFile);
        printf("%s\n", "ENTRO");
        bytesSended = resultSendFile;
      }

      // int bs, scount = 0;
      // while ((bs = sendfile(socketTcpFd, fileFd, NULL, BUF_SIZE)) > 0) {
      //     scount += bs;
      //     printf("sended %d bytes ...\n", bs);
      // }

      printf("%s\n", "[CLIENT] file sended successfully");

      close(fileFd);
      sendInvalidCommand = false;
    } // End if simage

    if (strcmp(nameCommandBuf, R_IMAGE) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(R_IMAGE, sendBuffer);

      /*
       * El cliente le envia el comando rimage al servidor con el
       * que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, strlen(sendBuffer) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, EXIT) == 0) {
      strcat(sendBuffer, FIELD_TCP);
      strcat(sendBuffer, inputBuffer);

      displayDataSent(EXIT, sendBuffer);

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

      displayDataSent(PING, sendBuffer);

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

      displayDataSent(ID, sendBuffer);

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

      displayDataSent(CALL_TO, sendBuffer);

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

      displayDataSent(TAKE_CALL, sendBuffer);

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

      displayDataSent(S_AUDIO, sendBuffer);

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
  int resultRecv;

  char outputBuffer[BUF_SIZE];

  for(;;) {
    numRead = read(socketTcpFd, outputBuffer, BUF_SIZE);

    printf("[CLIENT RECEIVER] numRead: %d\n", numRead);

    if (numRead == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    if (strcmp(outputBuffer, S_IMAGE) == 0) {
      printf("%s\n", "ENTRO A S_IMAGE");
      off_t fileSize;

      // Lee el tamaño del archivo a recibir
      resultRecv = recv(socketTcpFd, &fileSize, sizeof(fileSize), 0);

      if (resultRecv == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
      }

      fileSize = ntohl(fileSize);

      printf("[CLIENT RECEIVER] File size: %ld\n", fileSize);

      char buffer[BUF_SIZE];
      char fileName[BUF_SIZE] = "image.jpg";
      char end[BUF_SIZE] = "END_IMAGE";

      int bytesReceived = 0;
      int flags = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
      int fileFd = open(fileName, O_CREAT | O_TRUNC | O_WRONLY, flags);

      printf("%s\n", "ANTES DE ENTRAR EN EL WHILE");

      while (bytesReceived < fileSize) {
        resultRecv = recv(socketTcpFd, buffer, BUF_SIZE, 0);

        if (resultRecv == -1) {
          perror("recv");
          exit(EXIT_FAILURE);
        }

        if (resultRecv == 0) {
          perror("0");
          exit(EXIT_FAILURE);
        }

        printf("%d\n", bytesReceived);
        printf("%d\n", resultRecv);
        printf("%s\n", "ENTRO");
        // printf("CONTENIDO DE buffer: %s\n", buffer);

        // if (strcmp(buffer, end) == 0) {
        //   printf("%s\n", "ENTRO EN IF (buffer == end)");
        //   break;
        // }

        write(fileFd, buffer, resultRecv);
        bytesReceived += resultRecv;

        // if (bytesReceived == fileSize) {
        //   break;
        // }

      } // End while

      resetBuffer(outputBuffer);

      printf("%s\n", "SALIO DEL WHILE");

      // while (bytesReceived != fileSize) {
      //   printf("%s\n", "ENTRO EN EL WHILE");
      //   resultRecv = recv(socketTcpFd, buffer, BUF_SIZE, 0);
      //
      //   if (resultRecv == -1) {
      //     perror("recv");
      //     exit(EXIT_FAILURE);
      //   }
      //
      //   write(fileFd, buffer, resultRecv);
      //   bytesReceived += resultRecv;
      //
      //   if (bytesReceived == fileSize) {
      //     printf("%s\n", "ENTRO EN IF bytesReceived == fileSize");
      //     break;
      //   }
      //
      //   if (strcmp(buffer, end) == 0) {
      //     printf("%s\n", "ENTRO EN IF buffer == end");
      //     break;
      //   }
      //
      //   printf("%s\n", "DENTRO DEL WHILE");
      // }

      close(fileFd);
      printf("%s\n", "Image file received");
    }

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

  for(;;) {
    len = sizeof(addr);
    numRead = recvfrom(socketUdpFd, outputBuffer, BUF_SIZE, 0, (struct sockaddr *) &addr, &len);

    if (numRead == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    printf("[CLIENT][UDP] Response from server: %s\n", outputBuffer);

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
