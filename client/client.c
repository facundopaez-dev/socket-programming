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

/*
 * Variable utilizada para la exclusion mutua de los
 * recursos compartidos por los hilos
 */
pthread_mutex_t lock;

/*
 * Estructura utilizada para el pasaje de parametros
 * a los hilos
 *
 * Cuando se ejecuta el programa cliente, en la
 * funcion main se crean dos sockets, uno TCP y
 * el otro UDP, y de cada uno de ellos se obtiene
 * un descriptor de archivo, los cuales son
 * almacenados en esta estructura para luego
 * pasarla a los hilos, y asi poder usar,
 * dentro de ellos los descriptores de archivo
 * de los sockets creados, para la comunicacion
 * entre este cliente y un servidor
 */
struct structparams {
  int socketTcpFd;
  int socketUdpFd;
};

void *sendRequest(void *args);

void *receiveTcpResponse(void *args);

void *receiveUdpResponse(void *args);

/**
 * Esta funcion se encarga de crear los tres hilos
 * del cliente, uno de ellos es para enviar solicitudes
 * a un servidor, otro es para recibir respuestas (en
 * modo TCP) de parte del servidor y otro es para
 * recibir respuestas (en modo UDP) de parte del
 * servidor
 *
 * @param  argc
 * @param  argv
 * @return
 */
int main(int argc, char *argv[]) {
  struct structparams params;
  params.socketTcpFd = getFdSocketTcp(IP, PORT);

  /*
   * Se le tiene que pasar el ip y puerto local, no los
   * datos del servidor
   */
  params.socketUdpFd = getFdSocketUdp(params.socketTcpFd);

  int resultSendThread;
  int resultReceiveTcpThread;
  int resultReceiveUdpThread;

  pthread_t sendRequestThread;
  pthread_t receiveResponseTcpThread;
  pthread_t receiveResponseUdpThread;

  // printf("Ejecucion de la funcion main\n", "");

  struct sockaddr_in addr;
  socklen_t len = sizeof(struct sockaddr_in);

  getsockname(params.socketUdpFd, (struct sockaddr *) &addr, &len);
  // printf("[CLIENT] Port: %d\n", ntohs(addr.sin_port));
  // printf("[CLIENT] IP adress: %s\n", inet_ntoa(addr.sin_addr));

  resultSendThread = pthread_create(&sendRequestThread, NULL, sendRequest, (void *) &params);
  resultReceiveTcpThread = pthread_create(&receiveResponseTcpThread, NULL, receiveTcpResponse, (void *) &params);
  resultReceiveUdpThread = pthread_create(&receiveResponseUdpThread, NULL, receiveUdpResponse, (void *) &params);

  if (resultSendThread != 0) {
    printf("ERROR: return code from pthread_create() is %i\n", resultSendThread);
    close(params.socketTcpFd);
    close(params.socketUdpFd);
    exit(EXIT_FAILURE);
  }

  if (resultReceiveTcpThread != 0) {
    printf("ERROR: return code from pthread_create() is %i\n", resultReceiveTcpThread);
    close(params.socketTcpFd);
    close(params.socketUdpFd);
    exit(EXIT_FAILURE);
  }

  if (resultReceiveUdpThread != 0) {
    printf("ERROR: return code from pthread_create() is %i\n", resultReceiveUdpThread);
    close(params.socketTcpFd);
    close(params.socketUdpFd);
    exit(EXIT_FAILURE);
  }

  /*
   * Esto evita que el programa cliente termine su ejecucion
   *
   * Esta instruccion hace que el hilo principal, espere
   * la terminacion del hilo que se le pasa como primer
   * argumento, para fianlizar su ejecucion
   */
  pthread_join(sendRequestThread, NULL);
} // End main

/**
 * Esta funcion en el programa cliente esta encargada de enviarle
 * solicitudes al servidor con el cual esta conectado este cliente
 *
 * @param  args [se utiliza una estructura como argumento de esta funcion]
 * @return
 */
void *sendRequest(void *args) {
  /*
   * Recordar que esta estructura contiene los descriptores
   * de archivo de los sockets TCP y UDP creados en la
   * funcion main()
   */
  struct structparams *params = (struct structparams *) args;
  int socketTcpFd = params -> socketTcpFd;
  int socketUdpFd = params -> socketUdpFd;

  int numRead;
  int numWrite;
  int idDepartment;

  /*
   * Esta variable se la utiliza para que el cliente,
   * le envie al servidor, un aviso de que se ingreso
   * texto que no coincide con ninguno de los comandos
   * definidos en el protocolo de comunicacion
   */
  bool sendInvalidCommand = true;

  /*
   * Este arreglo almacena lo que se ingresa por
   * teclado
   */
  char inputBuffer[BUF_SIZE];

  /*
   * Este arreglo almacena el nombre del comando
   * que se ingresa por el teclado
   */
  char nameCommandBuf[BUF_SIZE];

  /*
   * Este arreglo contiene el modo de operacion
   * (TCP o UDP), seguido del nombre del comando
   * y el ID del departamento (si es que se lo provee
   * por el teclado)
   */
  char sendBuffer[BUF_SIZE] = "";

  for(;;) {
    /*
     * Espera que por teclado se ingrese un comando
     */
    numRead = read(0, inputBuffer, BUF_SIZE);

    if (numRead == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    /*
     * Esto suprime el \n de la cadena ingresada por el teclado
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
     * arreglo de conexiones (este arreglo esta en el programa
     * servidor y se llama clients) es una celda del arreglo, y
     * por ende, es el indice de esa celda
     */
    sscanf(inputBuffer, "%s%d", nameCommandBuf, &idDepartment);

    /*
     * Es necesario borrar el contenido del buffer de envio, en caso
     * contrario el programa cliente mostrara por pantalla
     * resultados erroneos
     */
    resetBuffer(sendBuffer);

    /*
     * Si el comando es para TCP, se tiene que ejecutar
     * haciendo uso del protocolo TCP
     *
     * Los comandos para las luces, el riego, la imagen,
     * la desconexion, el ping y el id hacen uso de TCP
     */

    /*
     * Se comprueba que lo ingresado por el teclado sea
     * la cadena "turnon", la cual esta contenida en
     * la constante TURN_ON, y asi se hace con el
     * resto de los comandos
     */
    if (strcmp(nameCommandBuf, TURN_ON) == 0) {
      /*
       * Se coloca en el primer lugar de sendBuffer
       * el modo de operacion, en este caso TCP
       */
      strcat(sendBuffer, FIELD_TCP);

      /*
       * Luego de colocar el modo de operacion en
       * sendBuffer, se coloca lo que se ingreso
       * por el teclado, lo cual es el nombre
       * del comando seguido del ID de un
       * departamento en caso de que sea provisto
       */
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
     * Comandos que hace uso de UDP: Audio
     */
    if (strcmp(nameCommandBuf, S_AUDIO) == 0) {
      /*
       * Se coloca en el primer lugar de sendBuffer
       * el modo de operacion, en este caso UDP
       */
      strcat(sendBuffer, FIELD_UDP);

      /*
       * Luego de colocar el modo de operacion en
       * sendBuffer, se coloca lo que se ingreso
       * por el teclado, lo cual es el nombre
       * del comando seguido del ID de un
       * departamento en caso de que sea provisto
       */
      strcat(sendBuffer, inputBuffer);

      displayCommandExecuted(S_AUDIO);
      displayDataSent(sendBuffer);

      /*
       * El cliente le envia el comando sendaudio al servidor con el
       * que esta conectado
       */
      numWrite = write(socketTcpFd, sendBuffer, BUF_SIZE);

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
      socklen_t len = sizeof(struct sockaddr_in);
      struct sockaddr_in addr;

      /*
       * El cliente recibe los datos del servidor asociados al socket
       * UDP del mismo, los cuales son la direccion IP y el puerto
       */
      numRead = read(socketTcpFd, (void *) &addr, len);

      // printf("%s\n", "");
      // printf("%s\n", "[CLIENT] UDP data");
      // printf("[CLIENT] Port: %d\n", ntohs(addr.sin_port));
      // printf("[CLIENT] IP adress: %s\n", inet_ntoa(addr.sin_addr));

      struct sockaddr_in addrClient;

      /*
       * Obtiene los datos de este cliente asociados al descriptor
       * de archivo de un socket TCP
       */
      getsockname(socketTcpFd, (struct sockaddr *) &addrClient, &len);

      /*
       * Este cliente le envia al servidor (por TCP) sus datos (IP y
       * puerto) para que el servidor pueda comunicarse con este cliente
       * mediante el protocolo UDP
       */
      numWrite = write(socketTcpFd, (void *) &addrClient, len);

      if (numWrite == -1) {
        perror("write");
        exit(EXIT_FAILURE);
      }

      sendInvalidCommand = false;
    }

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

/**
 * Esta funcion tiene la responsabilidad de recibir lo que el
 * servidor (con el cual esta conectado este cliente) le envia
 * por TCP
 *
 * @param  args [se utiliza una estructura como argumento de esta funcion]
 * @return
 */
void *receiveTcpResponse(void *args) {
  struct structparams *params = (struct structparams *) args;

  int socketTcpFd = params -> socketTcpFd;
  int numRead;
  int numWrite;

  char outputBuffer[BUF_SIZE];

  for(;;) {
    numRead = read(socketTcpFd, outputBuffer, BUF_SIZE);

    if (numRead == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    /*
     * El cliente comprueba si lo recibido de parte del
     * servidor es igual a "sendaudio", valor que esta
     * contenido en la constante S_AUDIO
     */
    if (strcmp(outputBuffer, S_AUDIO) == 0) {
      // printf("[CLIENT][RECEIVER][TCP] Recibó la notificación de parte del servidor de que alguien quiere hablar conmigo mediante UDP\n", "");
      // printf("[CLIENT][RECEIVER][TCP] Lo que voy a hacer es enviarle al servidor mi IP y puerto\n", "");
      struct sockaddr_in addr;
      socklen_t len = sizeof(struct sockaddr_in);

      /*
       * Este cliente receptor obtiene sus datos (IP y puerto)
       * asociados a su socket TCP
       */
      int resultGetSockName = getsockname(socketTcpFd, (struct sockaddr *) &addr, &len);

      if (resultGetSockName == -1) {
        perror("getsockname");
        exit(EXIT_FAILURE);
      }

      // printf("[CLIENT][RECEIVER][TCP] Mis datos son:\n", "");
      // printf("[CLIENT][RECEIVER][TCP] Port: %d\n", ntohs(addr.sin_port));
      // printf("[CLIENT][RECEIVER][TCP] IP adress: %s\n", inet_ntoa(addr.sin_addr));

      /*
       * Este cliente receptor le envia al servidor (mediante el socket TCP)
       * su direccion IP y puerto
       */
      numWrite = write(socketTcpFd, (const void *) &addr, sizeof(addr));

      if (numWrite == -1) {
        perror("write");
        exit(EXIT_FAILURE);
      }

      // printf("[CLIENT][RECEIVER][TCP] IP y puerto enviados al servidor\n", "");
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

/**
 * Esta funcion tiene la responsabilidad de recibir lo que el
 * servidor (con el cual esta conectado este cliente) le envia
 * por UDP
 *
 * @param  args [se utiliza una estructura como argumento de esta funcion]
 * @return
 */
void *receiveUdpResponse(void *args) {
  struct structparams *params = (struct structparams *) args;

  socklen_t len = sizeof(struct sockaddr_in);
  struct sockaddr_in addr;

  int socketUdpFd = params -> socketUdpFd;
  int numRead;

  char outputBuffer[BUF_SIZE];

  // int resultGetSockName = getsockname(socketUdpFd, (struct sockaddr *) &addr, &len);
  //
  // printf("[CLIENT][UDP] Se ejecuta, en un hilo, la funcion receiveUdpResponse\n", "");
  // printf("[CLIENT] Port: %d\n", ntohs(addr.sin_port));
  // printf("[CLIENT] IP adress: %s\n", inet_ntoa(addr.sin_addr));
  //
  // if (resultGetSockName == -1) {
  //   perror("getsockname");
  //   exit(EXIT_FAILURE);
  // }

  // len = sizeof(addr);

  for(;;) {
    len = sizeof(struct sockaddr_in);
    numRead = recvfrom(socketUdpFd, outputBuffer, BUF_SIZE, 0, (struct sockaddr *) &addr, &len);

    if (numRead == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    printf("[CLIENT][RECEIVER][UDP] Response from server: %s\n", outputBuffer);
  } // End for

}
