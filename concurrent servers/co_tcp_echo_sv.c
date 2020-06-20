#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "namecommands.h"
#include "answers.h"
#include "commandsdefinitions.h"
#include "utildefinitions.h"

/* Este arreglo contiene los FD de cada socket usado por cada cliente */
int clients[CONNECTION_LIMIT];

/*
 * Variable global utilizada para enviar un mensaje
 * por defecto cuando el comando ingresado por el
 * cliente no coincide con ninguno de los que estan
 * disponibles
 */
bool sendDefaultMessage;

/*
 * Variable global utilizada para terminar la ejecucion
 * de un hilo, lo cual se tiene que hacer cunado un cliente
 * decide desconectarse del servidor, lo cual lo hace
 * haciendo la llamada remota al comando exit
 */
bool disconnection;

/*
 * TODO: Comentar
 */
pthread_mutex_t lock;

/*
 * Estructura utilizada para pasar, los datos necesarios
 * para manejar las conexiones, a los hilos
 */
struct structparams {
  int acceptfd;
  int socketudpfd;
  int *amountConnections;
};

// TODO: Crear archivo de configuracion que contenga el puerto y la IP

/* Handle a client request: copy socket input back to socket */
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

  /*
   * Este arreglo contiene la cadena de texto que sera enviada
   * a aquellos clientes que no puedan conectarse al servidor,
   * lo cual sucede cuando se llega al limite de conexiones
   * activas
   */
  char messageLimitConnextions[BUF_SIZE] = ANSWER_LIMIT_CONNECTIONS;

  int resultPthreadCreate;
  int sockettcpfd = getFdSocketTcp(argv[1], argv[2]);
  int acceptfd;
  int socketudpfd;

  socklen_t addrlen;
  struct sockaddr_in addr;
  struct structparams params;

  for (;;) {
    // printf("%s%i\n", "**", amountConnections);

    /*
     * Si la cantidad de conexiones es menor que el limite
     * de conexiones, entonces el servidor acepta una nueva
     * conexion
     */
    if (amountConnections < CONNECTION_LIMIT) {
      acceptfd = accept(sockettcpfd, NULL, NULL); /* Wait for connection */
      socketudpfd = getFdSocketUdp(argv[1], argv[2]);

      if (acceptfd == -1) {
        perror("Accept error: Could not accept");
        exit(EXIT_FAILURE);
      }

      // printf("%s%i\n", "Cantidad de conexiones: ", amountConnections);

      params.acceptfd = acceptfd;
      params.socketudpfd = socketudpfd;
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
        addClient(acceptfd, &amountConnections, lock, clients);
        printConnections(&amountConnections);
      }

      /*
       * En caso de que haya un error al crear un hilo, se tiene
       * que avisar de esto y ademas se tienen que cerrar los sockets
       * TCP y UDP abiertos previamente
       */
      if (resultPthreadCreate != 0) {
        printf("ERROR: return code from pthread_create() is %i\n", resultPthreadCreate);
        close(acceptfd);
        close(socketudpfd);
      }

    } // End if

    /*
     * Si la variable cantidad de conexiones es igual al
     * limite de conexiones, entonces se tiene que rechazar
     * al cliente que esta tratandose de conectar al servidor
     */
    if (amountConnections == CONNECTION_LIMIT) {
      acceptfd = accept(sockettcpfd, (struct sockaddr*) &addr, &addrlen);
      send(acceptfd, messageLimitConnextions, BUF_SIZE, 0);
      close(acceptfd);
    }

  } // End for

} // End main

/* Handle a client request: copy socket input back to socket */
void *handleRequest(void *args) {
  struct structparams *params = (struct structparams *) args;

  int acceptfd = params -> acceptfd;
  int socketudpfd = params -> socketudpfd;
  int idDepartment = 0;

  char buf[BUF_SIZE];
  char nameCommandBuf[BUF_SIZE];
  ssize_t numRead;

  pthread_mutex_lock(&lock);
  sendDefaultMessage = true;
  disconnection = false;
  pthread_mutex_unlock(&lock);

  // Creacion de socket UDP
  // int socketudpfd;
  // int budpfd;
  //
  // struct sockaddr sudpaddr;
  // socklen_t udpaddrlen;
  //
  // udpaddrlen = sizeof(sudpaddr);
  //
  // getsockname(acceptfd, &sudpaddr, &udpaddrlen);
  //
  // socketudpfd = socket(AF_INET, SOCK_DGRAM, 0);
  //
  // budpfd = bind(socketudpfd, (struct sockaddr*) &sudpaddr, udpaddrlen);

  // Usar funcion getSockName, se puede crear un socket UDP aca
  // el cual esta asociado al mismo puerto e IP (si se hace uso)
  // de la funcion getSockName

  while ((numRead = read(acceptfd, buf, BUF_SIZE)) > 0) {
    /*
     * Esto suprime el \n de la cadena enviada por el cliente
     * haciendo que las cadenas de texto que son iguales no
     * den como resultado (por la funcion strcmp) que son
     * diferentes
     */
    buf[numRead - 1] = '\0';
    sscanf(buf, "%s%d", nameCommandBuf, &idDepartment);

    // Si el comando es para TCP, ejecutar comandos de TCP
    // Imagen, luces, riego, desconexion, ping, id
    if (strcmp(nameCommandBuf, TURN_ON) == 0) {
      turnon(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    }

    if(strcmp(nameCommandBuf, TURN_OFF) == 0) {
      turnoff(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    }

    if(strcmp(nameCommandBuf, I_ENABLE) == 0) {
      ienable(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    }

    if(strcmp(nameCommandBuf, I_DISABLE) == 0) {
      idisable(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    }

    if(strcmp(nameCommandBuf, EXIT) == 0) {
      disconnect(acceptfd, &sendDefaultMessage, &disconnection, nameCommandBuf, params -> amountConnections, lock, clients);
    }

    if(strcmp(nameCommandBuf, PING) == 0) {
      ping(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    }

    if(strcmp(nameCommandBuf, ID) == 0) {
      id(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    }

    // Si el comando es para UDP, ejecutar comandos de UDP
    // Audio, llamada

    if (strcmp(nameCommandBuf, R_IMAGE) == 0) {
      rimage(socketudpfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    }

    if (strcmp(nameCommandBuf, TAKE_CALL) == 0) {
      takecall(socketudpfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    }

    if (strcmp(nameCommandBuf, R_AUDIO) == 0) {
      recaudio(socketudpfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    }

    // Ejecucion de cada comando
    // turnon(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    // turnoff(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    // ienable(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    // idisable(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    // rimage(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    // takecall(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    // recaudio(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    // id(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);
    // disconnect(acceptfd, &sendDefaultMessage, &disconnection, nameCommandBuf, params -> amountConnections, lock, clients);
    // ping(acceptfd, &sendDefaultMessage, nameCommandBuf, lock, clients);

    /*
     * Separa el nombre del comando del argumento, el cual
     * es el ID de un departamento
     *
     * Hay que recordar que el ID de un departamento en el
     * arreglo de conexiones es una celda del arreglo, y
     * por ende, es el indice de esa celda
     */
    // sscanf(buf, "%s%d", nameCommandBuf, &idDepartment);

    // simage(acceptfd, &sendDefaultMessage, nameCommandBuf, idDepartment, lock, clients);
    // callto(acceptfd, &sendDefaultMessage, nameCommandBuf, idDepartment, lock, clients);
    // sendaudio(acceptfd, &sendDefaultMessage, nameCommandBuf, idDepartment, lock, clients);

    pthread_mutex_lock(&lock);
    if (sendDefaultMessage) {
      invalidCommand(acceptfd, buf, clients);
    }

    if (!sendDefaultMessage) {
      /*
       * Cuando esta variable tiene el valor falso
       * tiene que ser reiniciada al valor verdadero,
       * en caso de no hacerlo se tendra un error
       * logico y el programa fallara en su funcionamiento
       *
       * sendDefaultMessage es asignada con el valor falso
       * por la ejecucion de los comandos, y es inicializada
       * con el valor falso antes de la ejecucion del ciclo
       */
      sendDefaultMessage = true;
    }

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

  if (numRead == -1) {
    perror("Error from read");
    exit(EXIT_FAILURE);
  }

  /*
   * Si el flujo de ejecucion llega hasta aca es porque el
   * cliente ha decidido desconectarse y por ende se tiene
   * que cerrar el socket asociado al valor de la variable
   * entera socketfd
   */
  close(acceptfd);

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
