#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

// Headers
#include "headers/answers.h"
#include "headers/commandsdefinitions.h"
#include "headers/modes.h"
#include "headers/namecommands.h"
#include "headers/notices.h"
#include "headers/utildefinitions.h"

// Constants
#define IP "127.0.0.1"
#define PORT "50001"

/* Este arreglo contiene el descriptor de archivo del socket TCP usado por cada cliente */
int clients[CONNECTION_LIMIT];

/* Este arreglo contiene el descriptor de archivo del socket UDP usado por cada cliente */
int clientsUdp[CONNECTION_LIMIT];

/*
 * Variable global utilizada para terminar la ejecucion
 * de un hilo, lo cual se tiene que hacer cunado un cliente
 * decide desconectarse del servidor, lo cual lo hace
 * haciendo la llamada remota al comando exit
 */
bool disconnection;

/*
 * Variable utilizada para la exclusion mutua
 */
pthread_mutex_t lock;

/*
 * Estructura utilizada para pasar, los datos necesarios
 * para manejar las conexiones, a los hilos
 */
struct structparams {
  int acceptFd;
  int socketUdpFd;
  int *amountConnections;
};

void *handleRequest(void *args);

int main(int argc, char *argv[]) {
  pthread_t thread;

  /*
   * Variable utilizada para manejar la cantidad de conexiones
   * activas
   */
  int amountConnections = 0;

  /*
   * Carga el arreglo clients con el valor -1 para borrar datos
   * basura que hayan sido cargados en el al momento de crearlo
   */
  fill(clients);
  fill(clientsUdp);

  /*
   * Este arreglo contiene la cadena de texto que sera enviada
   * a aquellos clientes que no puedan conectarse al servidor,
   * lo cual sucede cuando se llega al limite de conexiones
   * activas
   */
  char messageLimitConnextions[BUF_SIZE] = ANSWER_LIMIT_CONNECTIONS;

  int resultPthreadCreate;
  int socketTcpFd = getFdSocketTcp(IP, PORT);
  int acceptFd;
  int socketUdpFd;

  socklen_t addrlen;
  struct sockaddr_in addr;
  struct structparams params;

  for (;;) {
    /*
     * Si la cantidad de conexiones es menor que el limite
     * de conexiones, entonces el servidor acepta una nueva
     * conexion
     */
    if (amountConnections < CONNECTION_LIMIT) {
      acceptFd = accept(socketTcpFd, NULL, NULL); /* Wait for connection */
      socketUdpFd = getFdSocketUdp(acceptFd);

      if (acceptFd == -1) {
        perror("Accept error: Could not accept");
        exit(EXIT_FAILURE);
      }

      params.acceptFd = acceptFd;
      params.socketUdpFd = socketUdpFd;
      params.amountConnections = &amountConnections;

      /* Handle each client request in a new thread */
      resultPthreadCreate = pthread_create(&thread, NULL, handleRequest, (void *) &params);

      /*
       * Un 0 como retorno de la llamada al sistema quiere decir que
       * un hilo se creo satisfactoriamente, en este caso, el hilo
       * principal añade el FD del cliente conectado al arreglo de
       * conexiones
       */
      if (resultPthreadCreate == 0) {
        addClient(acceptFd, socketUdpFd, &amountConnections, lock, clients, clientsUdp);
        printConnections(&amountConnections);
      }

      /*
       * En caso de que haya un error al crear un hilo, se tiene
       * que avisar de esto y ademas se tienen que cerrar los sockets
       * TCP y UDP abiertos previamente
       */
      if (resultPthreadCreate != 0) {
        printf("ERROR: return code from pthread_create() is %i\n", resultPthreadCreate);
        close(acceptFd);
        close(socketUdpFd);
      }

    } // End if

    /*
     * Si la variable cantidad de conexiones es igual al
     * limite de conexiones, entonces se tiene que rechazar
     * al cliente que esta tratandose de conectarse al servidor
     */
    if (amountConnections == CONNECTION_LIMIT) {
      acceptFd = accept(socketTcpFd, (struct sockaddr*) &addr, &addrlen);
      send(acceptFd, messageLimitConnextions, BUF_SIZE, 0);
      close(acceptFd);
    }

  } // End for

} // End main

/**
 * Esta funcion tiene la responsabilidad de manejar las solicitudes
 * de los clientes conectados a este servidor
 *
 * @param  args [se utiliza una estructura como argumento de esta funcion]
 * @return
 */
void *handleRequest(void *args) {
  struct structparams *params = (struct structparams *) args;

  int acceptFd = params -> acceptFd;
  int socketUdpFd = params -> socketUdpFd;
  int idDepartment = 0;
  int resultWrite = 0;

  char buf[BUF_SIZE] = "";

  /*
   * Se utiliza para almacenar la cadena de
   * caracteres que representa el modo de
   * operacion: TCP, UDP
   */
  char mode[BUF_SIZE] = "";

  /*
   * Se utiliza para almacenar el nombre del
   * comando ejecutado por un cliente
   */
  char nameCommandBuf[BUF_SIZE] = "";

  ssize_t numWrite = 0;
  ssize_t numRead = 0;

  pthread_mutex_lock(&lock);
  disconnection = false;
  pthread_mutex_unlock(&lock);

  while ((numRead = read(acceptFd, buf, BUF_SIZE)) > 0) {
    /*
     * Luego de cada ciclo esta variable tiene que ser
     * reiniciada, porque en el caso contrario hace que
     * el servidor envie respuestas erroneas a los clientes
     * que envian como argumento (en aquellos comandos que
     * usen argumento) un numero de departamento
     */
    idDepartment = 0;

    /*
     * Esto suprime el \n de la cadena enviada por el cliente
     * haciendo que las cadenas de texto que son iguales no
     * den como resultado (por la funcion strcmp) que son
     * diferentes
     */
    buf[numRead - 1] = '\0';

    printf("[SERVER] Buffer content: %s\n", buf);

    /*
     * Separa el nombre del comando del argumento, el cual
     * es el ID de un departamento
     *
     * Hay que recordar que el ID de un departamento en el
     * arreglo de conexiones es una celda del arreglo, y
     * por ende, es el indice de esa celda
     */
    sscanf(buf, "%s%s%i", mode, nameCommandBuf, &idDepartment);
    // printf("%s\n", nameCommandBuf);
    // printf("%d\n", idDepartment);

    if (strcmp(mode, TCP_MODE) == 0) {
      printf("%s\n", "[SERVER] TCP protocol in use");

      /*
       * Si el comando es para TCP, se tiene que ejecutar
       * haciendo uso del protocolo TCP
       *
       * Comandos: Luces, riego, imagen, desconexion,
       * llamada, ping y id
       */
      if (strcmp(nameCommandBuf, TURN_ON) == 0) {
        turnon(acceptFd, clients);
      }

      if(strcmp(nameCommandBuf, TURN_OFF) == 0) {
        turnoff(acceptFd, clients);
      }

      if(strcmp(nameCommandBuf, I_ENABLE) == 0) {
        ienable(acceptFd, clients);
      }

      if(strcmp(nameCommandBuf, I_DISABLE) == 0) {
        idisable(acceptFd, clients);
      }

      if (strcmp(nameCommandBuf, R_IMAGE) == 0) {
        rimage(acceptFd, clients);
      }

      if(strcmp(nameCommandBuf, EXIT) == 0) {
        disconnect(acceptFd, &disconnection, params -> amountConnections, lock, clients, clientsUdp);
      }

      if(strcmp(nameCommandBuf, PING) == 0) {
        ping(acceptFd, clients);
      }

      if(strcmp(nameCommandBuf, ID) == 0) {
        id(acceptFd, clients);
      }

      if (strcmp(nameCommandBuf, CALL_TO) == 0 ) {
        callto(acceptFd, idDepartment, clients);
      }

      if (strcmp(nameCommandBuf, TAKE_CALL) == 0) {
        takecall(acceptFd, clients);
      }

    } // End if TCP_MODE

    if (strcmp(mode, UDP_MODE) == 0) {
      printf("%s\n", "[SERVER] UDP protocol in use");
      // printf("[SERVER] UDP socket's number: %d\n", socketUdpFd);
      // printf("%s\n", "");

      /*
       * Obtiene los datos del servidor asociados al descriptor
       * de archivo de un socket UDP
       */
      struct sockaddr_in addrServer = getDataServer(socketUdpFd);

      // printf("%s\n", "[SERVER] Data server send to client");
      // printf("[SERVER] Puerto: %d\n", ntohs(addrServer.sin_port));
      // printf("[SERVER] Direccion: %s\n", inet_ntoa(addrServer.sin_addr));
      // printf("%s\n", "");

      /*
       * El servidor le envia al cliente (mediante el socket TCP) emisor
       * los datos del servidor asociados al socket UDP
       */
      numWrite = write(acceptFd, (const void *) &addrServer, sizeof(addrServer));

      if (numWrite == -1) {
        perror("write");
        exit(EXIT_FAILURE);
      }

      struct sockaddr_in addrSender;

      /*
       * El servidor recibe los datos del cliente emisor, los cuales
       * son la direccion IP y puerto
       */
      numRead = read(acceptFd, (void *) &addrSender, sizeof(struct sockaddr_in));

      // printf("[SERVER] Puerto emisor: %d\n", ntohs(addrSender.sin_port));
      // printf("[SERVER] Direccion emisor: %s\n", inet_ntoa(addrSender.sin_addr));

      if (numRead == -1) {
        perror("read");
        exit(EXIT_FAILURE);
      }

      /*
       * Si el comando es para UDP, se tiene que ejecutar
       * haciendo uso del protocolo UDP
       *
       * Comandos: Audio
       */
      // if (strcmp(nameCommandBuf, S_AUDIO) == 0) {
      //   printf("[SERVER] Command executed: %s\n", S_AUDIO);

        /*
         * 1. Avisarle por TCP al cliente receptor que alguien
         * quiere hablar con el
         *
         * 2. Esperar del cliente receptor sus datos, esto es,
         * su direccion IP y puerto
         *
         * 3. Una vez que se recibieron los datos del cliente
         * receptor, pasarlos como argumento en la invocacion
         * de la funcion sendaudio
         */

        // int receiverTcpFd = clients[idDepartment - 1];

        /*
         * 1. Se le avisa al cliente receptor de que alguien quiere
         * hablar con el, esto lo hace para poder recibir los datos
         * del cliente receptor
         */
        // resultWrite = write(receiverTcpFd, S_AUDIO, BUF_SIZE);
        //
        // if (resultWrite == -1) {
        //   perror("write");
        //   exit(EXIT_FAILURE);
        // }

        // struct sockaddr_in addrReceiver;

        /*
         * 2. El servidor recibe los datos del cliente receptor, los cuales
         * son la direccion IP y puerto
         */
      //   numRead = read(receiverTcpFd, (void *) &addrReceiver, sizeof(struct sockaddr_in));
      //
      //   if (numRead == -1) {
      //     perror("read");
      //     exit(EXIT_FAILURE);
      //   }
      //
      //   sendAudio(socketUdpFd, idDepartment, addrSender, addrReceiver, clientsUdp);
      // }

    } // End if UDP_MODE

    /*
     * Si lo recibido por el servidor de parte del cliente
     * emisor contiene la cadena de caracteres "invalid command", el
     * servidor le envia al cliente emisor el mensaje
     * "invalid command", siendo dicho cliente el que
     * causo dicho envio
     */
    if (strcmp(buf, INVALID_COMMAND_NOTICE) == 0) {
      invalidCommand(acceptFd, clients);
    }

    pthread_mutex_lock(&lock);

    /*
     * Si un cliente en un hilo ejecuto el comando exit
     * (solicitud de desconexion), el ciclo se tiene que interrumpir,
     * lo cual se hace mediante la variable disconnection
     * cuando tiene el valor verdadero, valor que es establecido
     * cuando un cliente ejecuta el comando exit
     *
     * Despues de esto, el socket y el hilo asociados a un
     * cliente que solicito desconectarse (lo cual se hace
     * mediante el comando exit) se tienen que cerrar y terminar
     * respectivamente
     */
    if (disconnection) {
      pthread_mutex_unlock(&lock);
      break;
    }

    pthread_mutex_unlock(&lock);

    /*
     * NOTE: No borrar, observacion de un error logico
     * Colocar esta instruccion aca esta mal porque previamente
     * se tiene la instruccion break, la cual se ejecuta si el valor
     * de la variable disconnection es verdadero, haciendo que se rompa
     * el bucle, lo cual a su vez hace que el flujo de ejecucion del
     * programa salte a la primera instruccion que hay despues del bucle,
     * con lo cual nunca sera ejecutada disconnection = false, y por ende,
     * nunca se va a poder cambiar su valor de verdadero a falso
     *
     * En conclusion, si se ejecuta la instruccion break (que sucede
     * cuando el valor de disconnection es verdadero), la instruccion
     * disconnection = false nunca se ejecuta, solo se ejecuta
     * en cada repeticion del ciclo cuando el valor de disconnection
     * es falso (ver el if que esta arriba de este comentario), lo
     * cual no sirve de nada porque al haber un break de esta manera
     * (es decir, antes), que se ejecuta cuando disconnection es verdadera
     * nunca se va a poder cambiar el valor de disconnection de verdadero
     * a falso
     *
     * En otras palabras, se le va a asignar el valor falso a la variable
     * disconnection en cada repeticion, lo cual no sirve ya que cuando
     * tiene el valor verdadero se rompe el ciclo y, en consecuencia, no
     * se le puede cambiar el valor de verdadero a falso
     */
    // disconnection = false;
  } // End while

  /*
   * Cuando esta variable tiene el valor verdadero
   * tiene que ser reiniciada al valor false,
   * en caso de no hacerlo se tendra un error
   * logico y el servidor no funcionara de
   * forma adecuada
   *
   * Esto es programacion concurrente con hilos, y los
   * hilos comparten el mismo espacio de memoria del mismo
   * proceso que los crea. Si un hilo establece esta
   * variable global en verdadero, los demas hilos tambien van
   * a tener el mismo valor de la variable, lo que va a
   * hacer que terminen de ejecutarse cuando en realidad solo
   * se tiene que terminar de ejecutarse el hilo que
   * la establecio con el valor verdadero.
   */
  pthread_mutex_lock(&lock);
  disconnection = false;
  pthread_mutex_unlock(&lock);

  /*
   * Si el flujo de ejecucion del programa servidor llega hasta
   * aca es porque el cliente ha decidido desconectarse y por
   * ende se tienen que cerrar los sockets TCP y UDP (abiertos)
   * asociados al valor de las variables enteras enviadas como
   * argumentos a la funcion close
   */
  close(acceptFd);
  close(socketUdpFd);

  /*
   * Hace que el hilo invocante deje de ejecutarse, es
   * decir, que finalice su ejecucion
   *
   * Esto se ejecuta cuando el cliente decide desconectarse
   * del servidor, lo cual se realiza mediante la llamada
   * remota al comando exit, y exit (disconnect en el codigo
   * fuente) hace que la variable boleana disconnection tome
   * el valor verdadero, lo cual hace que la instruccion de
   * repeticion while salga de su ciclo de repeticion,
   * haciendo que la instruccion pthread_exit() sea ejecutada
   */
  pthread_exit(NULL);
}
