#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include "../concurrent servers/namecommands.h"
#include "../concurrent servers/confirmations.h"
#include "../concurrent servers/notices.h"
#define BUF_SIZE 4096

pthread_mutex_t lock;

struct structparams {
  int socketTcpFd;
  int socketUdpFd;
};

int getFdSocketTcp(char* ip, char* port);

int getFdSocketUdp(char* ip, char* port);

void *sendRequest(void *args);

void *receiveResponse(void *args);

int main(int argc, char *argv[]) {
  int socketTcpFd = getFdSocketTcp(argv[1], argv[2]);
  int socketUdpFd = getFdSocketUdp(argv[1], argv[2]);

  int resultSendThread;
  int resultReceiveThread;

  pthread_t sendRequestThread;
  pthread_t receiveResponseThread;

  struct structparams params;
  params.socketTcpFd = socketTcpFd;
  params.socketUdpFd = socketUdpFd;

  resultSendThread = pthread_create(&sendRequestThread, NULL, sendRequest, (void *) &params);
  resultReceiveThread = pthread_create(&receiveResponseThread, NULL, receiveResponse, (void *) &params);

  if (resultSendThread != 0) {
    printf("ERROR: return code from pthread_create() is %i\n", resultSendThread);
    close(socketTcpFd);
    close(socketUdpFd);
    exit(EXIT_FAILURE);
  }

  if (resultReceiveThread != 0) {
    printf("ERROR: return code from pthread_create() is %i\n", resultReceiveThread);
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

int getFdSocketTcp(char* ip, char* port) {
  int socketTcpFd;
  int connectResult;

  socklen_t addrTcpLen;
  struct sockaddr_in addrTcp;

  addrTcpLen = sizeof(addrTcp);

  inet_aton(ip, &(addrTcp.sin_addr));
  addrTcp.sin_port = htons(atoi(port));
  addrTcp.sin_family = AF_INET;

  /*
   * AF_INET y SOCK_STREAM indican que este socket
   * usa IPv4 y flujos respectivamente para hacer
   * la comunicacion entre un cliente y el servidor
   */
  socketTcpFd = socket(AF_INET, SOCK_STREAM, 0);

  if (socketTcpFd == -1) {
    perror("Socket error: Could not create server socket");
    exit(EXIT_FAILURE);
  }

  int optval = 1;

  // Esto indica que el puerto esta marcado para reutilizar
  if(setsockopt(socketTcpFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  connectResult = connect(socketTcpFd, (struct sockaddr*) &addrTcp, addrTcpLen);

  if (connectResult == -1) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  return socketTcpFd;
}

int getFdSocketUdp(char* ip, char* port) {
  int socketUdpFd;
  int bindUdpFd;

  struct sockaddr_in addrUdp;
  socklen_t addrlenUdp;

  addrlenUdp = sizeof(addrUdp);

  inet_aton(ip, &(addrUdp.sin_addr));
  addrUdp.sin_port = htons(atoi(port));

  /* La constante AF_INET indica que la comunicacion entre los clientes
  y el servidor (ambos son aplicaciones) va a ser utilizando el protocolo IPv4 */
  addrUdp.sin_family = AF_INET;

  /* La constante SOCK_DGRAM sirve para indicar que se va a crear un
  socket de datagramas (servicio no orientado a la conexion) */
  socketUdpFd = socket(AF_INET, SOCK_DGRAM, 0);

  if (socketUdpFd == -1) {
    perror("Socket error: Could not create server socket");
    exit(EXIT_FAILURE);
  }

  int optval = 1;

  // Esto indica que el puerto esta marcado para reutilizar
  if(setsockopt(socketUdpFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  bindUdpFd = bind(socketUdpFd, (struct sockaddr*) &addrUdp, addrlenUdp);

  if (bindUdpFd == -1) {
    perror("Bind error: Could not bind server socket");
    exit(EXIT_FAILURE);
  }

  return socketUdpFd;
}

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
  char outputBuffer[BUF_SIZE];

  FILE* handlerFile = NULL;

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
    * Si el comando es para TCP, se tiene que ejecutar
    * haciendo uso del protocolo TCP
    *
    * Comandos: Luces, riego, imagen, desconexion,
    * ping y id
    */
    if (strcmp(nameCommandBuf, TURN_ON) == 0) {
      numWrite = write(socketTcpFd, nameCommandBuf, strlen(nameCommandBuf) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, TURN_OFF) == 0) {
      numWrite = write(socketTcpFd, nameCommandBuf, strlen(nameCommandBuf) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, I_ENABLE) == 0) {
      numWrite = write(socketTcpFd, nameCommandBuf, strlen(nameCommandBuf) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, I_DISABLE) == 0) {
      numWrite = write(socketTcpFd, nameCommandBuf, strlen(nameCommandBuf) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, S_IMAGE) == 0) {
      /*
       * Se le avisa al servidor que se quiere ejecutar
       * el comando simage
       */
      numWrite = write(socketTcpFd, inputBuffer, strlen(inputBuffer) + 1);

      if (numWrite == -1) {
        perror("write");
        exit(EXIT_FAILURE);
      }

      numRead = read(socketTcpFd, outputBuffer, BUF_SIZE);

      if (numRead == -1) {
        perror("read");
        exit(EXIT_FAILURE);
      }

      /*
       * En caso de que el cliente receptor este disponible para la
       * transmision del archivo, se la inicia
       */
      if (strcmp(outputBuffer, AVAILABLE_CUSTOMER_CONFIRMATION) == 0) {
        // Abrir el archivo
        handlerFile = fopen("./file.txt", "r");

        if (handlerFile == NULL) {
          perror("fopen");
          exit(EXIT_FAILURE);
        }

        // Transmitir sus bytes hasta el ultimo
        while (!feof(handlerFile)) {
          fgets(inputBuffer, BUF_SIZE, handlerFile);
          numWrite = write(socketTcpFd, inputBuffer, BUF_SIZE);

          // send(socketTcpFd, inputBuffer, BUF_SIZE, 0);

          if (numWrite == -1) {
            perror("write");
            exit(EXIT_FAILURE);
          }

        }

        char confirmation[BUF_SIZE] = EOF_CONFIRMATION;
        write(socketTcpFd, confirmation, BUF_SIZE);

        // Cerrar el archivo
        fclose(handlerFile);
      }

      if (strcmp(outputBuffer, AVAILABLE_CUSTOMER_CONFIRMATION) != 0) {
        printf("El cliente %i no está disponible para la transmisión del archivo\n", idDepartment);
      }

      // exit(EXIT_SUCCESS);

      // numWrite = write(socketTcpFd, inputBuffer, strlen(inputBuffer) + 1);
      sendInvalidCommand = false;
    } // End if simage

    if (strcmp(nameCommandBuf, R_IMAGE) == 0) {
      numWrite = write(socketTcpFd, nameCommandBuf, strlen(inputBuffer) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, EXIT) == 0) {
      numWrite = write(socketTcpFd, nameCommandBuf, strlen(nameCommandBuf) + 1);
      sendInvalidCommand = false;
      exit(EXIT_SUCCESS);
    }

    if(strcmp(nameCommandBuf, PING) == 0) {
      numWrite = write(socketTcpFd, nameCommandBuf, strlen(nameCommandBuf) + 1);
      sendInvalidCommand = false;
    }

    if(strcmp(nameCommandBuf, ID) == 0) {
      numWrite = write(socketTcpFd, nameCommandBuf, strlen(nameCommandBuf) + 1);
      sendInvalidCommand = false;
    }

    /*
    * Si el comando es para UDP, se tiene que ejecutar
    * haciendo uso del protocolo UDP
    *
    * Comandos: Audio, llamada
    */
    if (strcmp(nameCommandBuf, S_AUDIO) == 0) {
      // len = sizeof(struct sockaddr_storage);
      // sendto(socketUdpFd, inputBuffer, numRead, 0, (struct sockaddr *) &claddr, len);
      numWrite = write(socketTcpFd, inputBuffer, strlen(inputBuffer) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, R_AUDIO) == 0) {
      numWrite = write(socketTcpFd, nameCommandBuf, strlen(nameCommandBuf) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, CALL_TO) == 0 ) {
      numWrite = write(socketTcpFd, inputBuffer, strlen(inputBuffer) + 1);
      sendInvalidCommand = false;
    }

    if (strcmp(nameCommandBuf, TAKE_CALL) == 0) {
      numWrite = write(socketTcpFd, nameCommandBuf, strlen(nameCommandBuf) + 1);
      sendInvalidCommand = false;
    }

    if (sendInvalidCommand) {
      numWrite = write(socketTcpFd, inputBuffer, strlen(inputBuffer) + 1);
    }

    if (!sendInvalidCommand) {
      sendInvalidCommand = true;
    }

    if (numWrite == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    }

  } // End for

}

void *receiveResponse(void *args) {
  struct structparams *params = (struct structparams *) args;

  int numRead;
  int socketTcpFd = params -> socketTcpFd;
  int socketUdpFd = params -> socketUdpFd;

  char outputBuffer[BUF_SIZE];

  FILE* newFile = NULL;

  for(;;) {
    numRead = read(socketTcpFd, outputBuffer, BUF_SIZE);

    if (numRead == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    /*
     * Si el cliente receptor recibe una notificacion de que
     * un cliente emisor le envia un archivo tiene que
     * recibirlo
     */
    if (strcmp(outputBuffer, FILE_SEND_NOTIFICATION) == 0) {
      // printf("%s\n", "DENTRO DEL IF EN EL CLIENTE RECEPTOR");

      // Abrir/crear archivo
      newFile = fopen("newFile.txt", "w");

      while ((numRead = read(socketTcpFd, outputBuffer, BUF_SIZE)) > 0) {
        // printf("%s\n", "DENTRO WHILE EN EL CLIENTE RECEPTOR");
        fputs(outputBuffer, newFile);

        printf("%s\n", "DENTRO DEL WHILE");

        if (strcmp(outputBuffer, EOF_CONFIRMATION) == 0) {
          break;
        }

      }

      // Cerrar el archivo creado
      fclose(newFile);
    }

    printf("%s\n", outputBuffer);
  }

}
