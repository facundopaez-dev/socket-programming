#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <ctype.h>
#include "namecommands.h"
#include "answers.h"
#define FREE_CONNECTION -1
#define BUF_SIZE 4096 /* Maximo tamaño del bufer para los flujos de datos que pueden leer el cliente y el servidor */
#define CONNECTION_LIMIT 10

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
  int *amountConnections;
};

// Utilidades
static int responseToClient(int acceptfd, char* answer);
static void sendNotice(int acceptfd, int receivingDepartmentId, char* answer);
static void responseToServer(int acceptfd, char* command, char* answer, int resultWrite);
static void responseInvalidCommand(int acceptfd, char* answer, char* invalidCommand);
static bool existDepartment(int idDepartment);
static bool connectedClient(int idDepartment);
static bool equalDepartment(int senderfd, int idDepartment);
static int getIdDepartment(int acceptfd);
static void invalidCommand(int acceptfd, char* invalidCommand);
static void printConnections(int *amountConnections);

// Comandos
static void turnon(int acceptfd, const char* buf);
static void turnoff(int acceptfd, const char* buf);
static void ienable(int acceptfd, const char* buf);
static void idisable(int acceptfd, const char* buf);
static void rimage(int acceptfd, const char* buf);
static void takecall(int acceptfd, const char* buf);
static void recaudio(int acceptfd, const char* buf);
static void id(int acceptfd, const char* buf);
static void ping(int acceptfd, const char* buf);
static void simage(int acceptfd, const char* buf, int idDepartment);
static void callto(int acceptfd, const char* buf, int idDepartment);
static void sendaudio(int acceptfd, const char* buf, int idDepartment);

/**
 * Registra las conexiones que hay entre los clientes y el servidor, si
 * el servidor no ha llegado a su limite de conexiones activas
 *
 * @param  clients           [arreglo de clientes o conexiones]
 * @param  connectionfd      [FD del socket usado para la conexion entre el servidor y un cliente]
 * @param  amountConnections [cantidad de conexiones activas]
 * @return cantidad de conexiones activas
 */

// TODO: Modificar la descripcion
static void addClient(int connectionfd, int *amountConnections) {
  pthread_mutex_lock(&lock);

  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {

    /*
     * Si la celda i del arreglo de clientes o conexiones
     * esta libre, entonces se puede establecer la conexion
     * entre el servidor y un cliente, y se registra esta
     * conexion en la celda i del arreglo de clientes
     *
     * Una celda se considera libre si tiene el valor -1
     */
    if (clients[i] == FREE_CONNECTION) {
      /*
       * Registra el descriptor de archivo del socket
       * a traves del cual se hace la comunicacion
       * entre el hilo y un cliente
       */
      clients[i] = connectionfd;

      /*
       * Se incrementa la cantidad de conexiones activas
       */
      (*amountConnections) = (*amountConnections) + 1;

      /*
       * Una vez que se encontro un espacio libre en el arreglo
       * se tiene que cortar el ciclo
       */
      break;
    }

  } // End for

  pthread_mutex_unlock(&lock);
}

// TODO: Documentar
static void removeClient(int clients[], int connectionfd, int *amountConnections) {

  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {

    if (clients[i] == connectionfd) {
      clients[i] = FREE_CONNECTION;
      (*amountConnections) = (*amountConnections) - 1;
      break;
    }

  }

}

static void disconnect(int acceptfd, const char* buf, int *amountConnections) {

  if (strcmp(buf, EXIT) == 0) {
    int resultWrite = responseToClient(acceptfd, ANSWER_SENDER_EXIT);
    responseToServer(acceptfd, EXIT, ANSWER_SENDER_EXIT, resultWrite);

    pthread_mutex_lock(&lock);
    removeClient(clients, acceptfd, amountConnections);
    sendDefaultMessage = false;
    disconnection = true;
    pthread_mutex_unlock(&lock);

    close(acceptfd);
    printConnections(amountConnections);
  }

}

// TODO: Crear archivo de configuracion que contenga el puerto y la IP

/* Handle a client request: copy socket input back to socket */
void *handleRequest(void *args) {
  struct structparams *params = (struct structparams *) args;

  int acceptfd = params -> acceptfd;
  int idDepartment;

  char buf[BUF_SIZE];
  char nameCommandBuf[BUF_SIZE];
  ssize_t numRead;

  pthread_mutex_lock(&lock);
  sendDefaultMessage = true;
  disconnection = false;
  pthread_mutex_unlock(&lock);

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

    // Ejecucion de cada comando
    turnon(acceptfd, buf);
    turnoff(acceptfd, buf);
    ienable(acceptfd, buf);
    idisable(acceptfd, buf);
    rimage(acceptfd, buf);
    takecall(acceptfd, buf);
    recaudio(acceptfd, buf);
    id(acceptfd, buf);
    disconnect(acceptfd, buf, params -> amountConnections);
    ping(acceptfd, buf);

    sscanf(buf, "%s%d", nameCommandBuf, &idDepartment);

    simage(acceptfd, nameCommandBuf, idDepartment);
    callto(acceptfd, nameCommandBuf, idDepartment);
    sendaudio(acceptfd, nameCommandBuf, idDepartment);

    pthread_mutex_lock(&lock);
    if (sendDefaultMessage) {
      invalidCommand(acceptfd, buf);
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

/**
 * Carga el arreglo dado con el vañor -1 para indicar
 * que no hay clientes conectados al servidor, o en
 * otras palabras que no hay conexiones abiertas
 *
 * @param clients [int array]
 */

// TODO: Modificar descripcion, y añadir el signifcado de -1
static void fill() {
  for (size_t j = 0; j < CONNECTION_LIMIT; j++) {
    clients[j] = -1;
  }
}

int main(int argc, char *argv[]) {
  pthread_t thread;

  int socketfd;
  int bindfd;
  int listenfd;
  int acceptfd;
  int resultPthreadCreate;

  /*
   * Variable utilizada para manejar la cantidad de conexiones
   * activas
   */
  int amountConnections = 0;

  /*
   * Carga el arreglo clients con el valor -1 para borrar datos
   * basura que hayan sido cargados en el al momento de crearlo
   */
  fill();

  ssize_t numRead;
  socklen_t addrlen, len;

  /*
   * Este arreglo contiene la cadena de texto que sera enviada
   * a aquellos clientes que no puedan conectarse al servidor,
   * lo cual sucede cuando se llega al limite de conexiones
   * activas
   */
  char messageLimitConnextions[BUF_SIZE] = ANSWER_LIMIT_CONNECTIONS;

  struct sockaddr_storage claddr;
  struct sockaddr_in addr;

  addrlen = sizeof(addr);

  inet_aton(argv[1], &(addr.sin_addr));
  addr.sin_port = htons(atoi(argv[2]));
  addr.sin_family = AF_INET; /* Constante que indica el uso de IPv4 */

  socketfd = socket(AF_INET, SOCK_STREAM, 0);

  if (socketfd == -1) {
    perror("Socket error: Could not create server socket");
    exit(EXIT_FAILURE);
  }

  int optval = 1;

  // Esto indica que el puerto esta marcado para reutilizar
  if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  bindfd = bind(socketfd, (struct sockaddr*) &addr, addrlen);

  if (bindfd == -1) {
    perror("Bind error: Could not bind server socket");
    exit(EXIT_FAILURE);
  }

  listenfd = listen(socketfd, CONNECTION_LIMIT);

  if (listenfd == -1) {
    perror("Could not listen");
    exit(EXIT_FAILURE);
  }

  struct structparams params;

  for (;;) {
    // printf("%s%i\n", "**", amountConnections);

    /*
     * Si la cantidad de conexiones es menor que el limite
     * de conexiones, entonces el servidor acepta una nueva
     * conexion
     */
    if (amountConnections < CONNECTION_LIMIT) {
      acceptfd = accept(socketfd, NULL, NULL); /* Wait for connection */

      if (acceptfd == -1) {
        perror("Accept error: Could not accept");
        exit(EXIT_FAILURE);
      }

      // printf("%s%i\n", "Cantidad de conexiones: ", amountConnections);

      params.acceptfd = acceptfd;
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
        addClient(acceptfd, &amountConnections);
        printConnections(&amountConnections);
      }

      /*
       * En caso de que haya un error al crear un hilo, se tiene
       * que avisar de esto y ademas se tiene que cerrar el socket
       * creado previamente
       */
      if (resultPthreadCreate != 0) {
        printf("ERROR: return code from pthread_create() is %i\n", resultPthreadCreate);
        close(acceptfd);
      }

    } // End if

    /*
     * Si la variable cantidad de conexiones es igual al
     * limite de conexiones, entonces se tiene que rechazar
     * al cliente que esta tratandose de conectar al servidor
     */
    if (amountConnections == CONNECTION_LIMIT) {
      acceptfd = accept(socketfd, (struct sockaddr*) &addr, &addrlen);
      send(acceptfd, messageLimitConnextions, BUF_SIZE, 0);
      close(acceptfd);
    }

  } // End for

} // End main

// TODO: Documentar
static int responseToClient(int acceptfd, char* answer) {
  return write(acceptfd, answer, strlen(answer) + 1);
}

static void responseToServer(int acceptfd, char* command, char* answer, int resultWrite) {

  if (resultWrite >= 0) {
    int senderDepartmentId = getIdDepartment(acceptfd);
    printf("The client of the department %i executes the command: %s\n", senderDepartmentId, command);
    printf("Response: %s\n", answer);
  }

}

static void invalidCommand(int acceptfd, char* invalidCommand) {
  responseInvalidCommand(acceptfd, ANSWER_INVALID_COMMAND, invalidCommand);
}

static void responseInvalidCommand(int acceptfd, char* answer, char* invalidCommand) {

  if (write(acceptfd, answer, strlen(answer) + 1) >= 0) {
    int senderDepartmentId = getIdDepartment(acceptfd);
    printf("The client of the department %i executes an invalid command\n", senderDepartmentId);
    printf("Invalid command: %s\n", invalidCommand);
    printf("Response: %s\n", answer);
  }

}

// TODO: Documentar
static void sendNotice(int acceptfd, int receivingDepartmentId, char* notice) {
  if (write(acceptfd, notice, strlen(notice) + 1) >= 0) {
    printf("Notification for customer %i: %s\n", receivingDepartmentId, notice);
  }
}

// TODO: Documentar
static void turnon(int acceptfd, const char* buf) {

  if (strcmp(buf, TURN_ON) == 0) {
    int resultWrite = responseToClient(acceptfd, ANSWER_SENDER_TURN_ON);
    responseToServer(acceptfd, TURN_ON, ANSWER_SENDER_TURN_ON, resultWrite);
    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

// TODO: Documentar
static void turnoff(int acceptfd, const char* buf) {

  if (strcmp(buf, TURN_OFF) == 0) {
    int resultWrite = responseToClient(acceptfd, ANSWER_SENDER_TURN_OFF);
    responseToServer(acceptfd, TURN_OFF, ANSWER_SENDER_TURN_OFF, resultWrite);
    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

// TODO: Documentar
static void ienable(int acceptfd, const char* buf) {

  if (strcmp(buf, I_ENABLE) == 0) {
    int resultWrite = responseToClient(acceptfd, ANSWER_SENDER_I_ENABLE);
    responseToServer(acceptfd, I_ENABLE, ANSWER_SENDER_I_ENABLE, resultWrite);
    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

// TODO: Documentar
static void idisable(int acceptfd, const char* buf) {

  if (strcmp(buf, I_DISABLE) == 0) {
    int resultWrite = responseToClient(acceptfd, ANSWER_SENDER_I_DISABLE);
    responseToServer(acceptfd, I_DISABLE, ANSWER_SENDER_I_DISABLE, resultWrite);
    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

// TODO: Documentar
static void simage(int senderfd, const char* buf, int idDepartment) {

  if (strcmp(buf, S_IMAGE) == 0) {
    int resultWrite;

    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);

    /*
     * Comprueba si el departamento solicitado
     * existe, en caso de que no exista se le avisa
     * de esto al cliente emisor de la imagen
     */
    if (!existDepartment(idDepartment)) {
      resultWrite = responseToClient(senderfd, ANSWER_NO_DEPARTMENT);
      responseToServer(senderfd, S_IMAGE, ANSWER_NO_DEPARTMENT, resultWrite);
      return;
    }

    // TODO: Modificar comentario
    /*
     * Si el departamento al que se quiere enviar algo
     * es igual al departamento del emisor, no se tiene
     * que enviar nada y se tiene que avisar de esto
     * al emisor
     */
    if (equalDepartment(senderfd, idDepartment)) {
      resultWrite = responseToClient(senderfd, ANSWER_EQUAL_DEPARTMENT);
      responseToServer(senderfd, S_IMAGE, ANSWER_EQUAL_DEPARTMENT, resultWrite);
      return;
    }

    /*
     * Si el departamento existe, se comprueba que
     * el cliente de dicho departamento este conectado,
     * en caso de que el cliente receptor no este conectado
     * se le avisa al cliente emisor de esto
     */
    if (!connectedClient(idDepartment)) {
      resultWrite = responseToClient(senderfd, ANSWER_CLIENT_NOT_CONNECTED);
      responseToServer(senderfd, S_IMAGE, ANSWER_CLIENT_NOT_CONNECTED, resultWrite);
      return;
    }

    // TODO: Modificar
    int senderDepartmentId = getIdDepartment(senderfd);
    int receiverfd = clients[idDepartment - 1];

    char destiny[BUF_SIZE] = IMAGE_NOTICE_TO_RECEIVER_FIRST_PART;
    char source[BUF_SIZE];
    source[0] = senderDepartmentId + '0';
    char secondPart[BUF_SIZE] = IMAGE_NOTICE_TO_RECEIVER_SECOND_PART;

    strcat(destiny, source);
    strcat(destiny, secondPart);

    /*
     * Si el flujo de ejecucion llega hasta aca, es porque el
     * cliente receptor (al que el cliente emisor le quiere
     * enviar una imagen), esta conectado, en este caso, al
     * cliente receptor se le tiene que enviar un aviso de
     * que alguien le envio una imagen
     */
    sendNotice(receiverfd, idDepartment, destiny);

    /*
     * Se le avisa al cliente emisor que su
     * imagen fue enviada satisfactoriamente
     * al cliente receptor
     */
    // response(senderfd, ANSWER_SENDER_S_IMAGE, S_IMAGE);

    resultWrite = responseToClient(senderfd, ANSWER_SENDER_S_IMAGE);
    responseToServer(senderfd, S_IMAGE, ANSWER_SENDER_S_IMAGE, resultWrite);
  }

}

// TODO: Documentar
static void rimage(int acceptfd, const char* buf) {

  if (strcmp(buf, R_IMAGE) == 0) {
    int resultWrite = responseToClient(acceptfd, ANSWER_RECEIVER_R_IMAGE);
    responseToServer(acceptfd, R_IMAGE, ANSWER_RECEIVER_R_IMAGE, resultWrite);
    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

// TODO: Documentar
static void takecall(int acceptfd, const char* buf) {

  if (strcmp(buf, TAKE_CALL) == 0) {
    int resultWrite = responseToClient(acceptfd, ANSWER_RECEIVER_TAKE_CALL);
    responseToServer(acceptfd, TAKE_CALL, ANSWER_RECEIVER_TAKE_CALL, resultWrite);
    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

// TODO: Documentar
static void callto(int senderfd, const char* buf, int idDepartment) {

  if (strcmp(buf, CALL_TO) == 0) {
    int resultWrite;

    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);

    // TODO: Documentar
    if (!existDepartment(idDepartment)) {
      resultWrite = responseToClient(senderfd, ANSWER_NO_DEPARTMENT);
      responseToServer(senderfd, CALL_TO, ANSWER_NO_DEPARTMENT, resultWrite);
      return;
    }

    // TODO: Documentar
    if (equalDepartment(senderfd, idDepartment)) {
      resultWrite = responseToClient(senderfd, ANSWER_EQUAL_DEPARTMENT);
      responseToServer(senderfd, CALL_TO, ANSWER_EQUAL_DEPARTMENT, resultWrite);
      return;
    }

    // TODO: Documentar
    if (!connectedClient(idDepartment)) {
      resultWrite = responseToClient(senderfd, ANSWER_CLIENT_NOT_CONNECTED);
      responseToServer(senderfd, CALL_TO, ANSWER_CLIENT_NOT_CONNECTED, resultWrite);
      return;
    }

    // TODO: Modificar
    int senderDepartmentId = getIdDepartment(senderfd);
    int receiverfd = clients[idDepartment - 1];

    char destiny[BUF_SIZE] = CALL_NOTICE_TO_RECEIVER_FIRST_PART;
    char source[BUF_SIZE];
    source[0] = senderDepartmentId + '0';
    char secondPart[BUF_SIZE] = CALL_NOTICE_TO_RECEIVER_SECOND_PART;

    strcat(destiny, source);
    strcat(destiny, secondPart);

    // TODO: Documentar
    sendNotice(receiverfd, idDepartment, destiny);

    // TODO: Documentar
    resultWrite = responseToClient(senderfd, ANSWER_SENDER_CALL_TO);
    responseToServer(senderfd, CALL_TO, ANSWER_SENDER_CALL_TO, resultWrite);
  }

}

// TODO: Documentar
static void sendaudio(int senderfd, const char* buf, int idDepartment) {

  if (strcmp(buf, S_AUDIO) == 0) {
    int resultWrite;

    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);

    // TODO: Documentar
    if (!existDepartment(idDepartment)) {
      resultWrite = responseToClient(senderfd, ANSWER_NO_DEPARTMENT);
      responseToServer(senderfd, S_AUDIO, ANSWER_NO_DEPARTMENT, resultWrite);
      return;
    }

    // TODO: Documentar
    if (equalDepartment(senderfd, idDepartment)) {
      resultWrite = responseToClient(senderfd, ANSWER_EQUAL_DEPARTMENT);
      responseToServer(senderfd, S_AUDIO, ANSWER_EQUAL_DEPARTMENT, resultWrite);
      return;
    }

    // TODO: Documentar
    if (!connectedClient(idDepartment)) {
      resultWrite = responseToClient(senderfd, ANSWER_CLIENT_NOT_CONNECTED);
      responseToServer(senderfd, S_AUDIO, ANSWER_CLIENT_NOT_CONNECTED, resultWrite);
      return;
    }

    // TODO: Modificar
    int senderDepartmentId = getIdDepartment(senderfd);
    int receiverfd = clients[idDepartment - 1];

    char destiny[BUF_SIZE] = CALL_NOTICE_TO_RECEIVER_FIRST_PART;
    char source[BUF_SIZE];
    source[0] = senderDepartmentId + '0';
    char secondPart[BUF_SIZE] = CALL_NOTICE_TO_RECEIVER_SECOND_PART;

    strcat(destiny, source);
    strcat(destiny, secondPart);

    // TODO: Documentar
    sendNotice(receiverfd, idDepartment, destiny);

    // TODO: Documentar
    resultWrite = responseToClient(senderfd, ANSWER_SENDER_SEND_AUDIO);
    responseToServer(senderfd, S_AUDIO, ANSWER_SENDER_SEND_AUDIO, resultWrite);
  }

}

// TODO: Documentar
static void recaudio(int acceptfd, const char* buf) {

  if (strcmp(buf, R_AUDIO) == 0) {
    int resultWrite = responseToClient(acceptfd, ANSWER_RECEIVER_REC_AUDIO);
    responseToServer(acceptfd, R_AUDIO, ANSWER_RECEIVER_REC_AUDIO, resultWrite);
    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

void resetCharArray(char buf[]) {
  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {
    buf[i] = '\0';
  }
}

// TODO: Documentar
static void id(int acceptfd, const char* buf) {

  if (strcmp(buf, ID) == 0) {
    char destiny[BUF_SIZE] = ANSWER_SENDER_ID;
    char source[BUF_SIZE];
    resetCharArray(source);

    int idDepartment = getIdDepartment(acceptfd);

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

    int resultWrite = responseToClient(acceptfd, destiny);
    responseToServer(acceptfd, ID, destiny, resultWrite);
    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

static void ping(int acceptfd, const char* buf) {

  if (strcmp(buf, PING) == 0) {
    int resultWrite = responseToClient(acceptfd, ANSWER_SENDER_PING);
    responseToServer(acceptfd, PING, ANSWER_SENDER_PING, resultWrite);
    pthread_mutex_lock(&lock);
    sendDefaultMessage = false;
    pthread_mutex_unlock(&lock);
  }

}

static void printConnections(int *amountConnections) {
  printf("%s%i\n", "Ammount connections: ", *amountConnections);
}

// TODO: Añadir descripcion
static bool existDepartment(int idDepartment) {
  /*
   * El ID del departamento es el numero de la posicion, del arreglo
   * de conexiones, en la cual se encuentra el descriptor de
   * archivo devuelto por la llamada al sistema accept al conectarse
   * un cliente al servidor
   *
   * Sabiendo lo anterior se puede determinar si existe el ID de un departamento,
   * enviado por otro cliente, verificando que este dentro del tamaño establecido
   * para el arreglo de conexiones, tamaño que es determinado por el valor de la
   * constante CONNECTION_LIMIT
   *
   * La verificacion se hace a partir de 1 porque si se la hace a partir de 0
   * se obtendra el valor -1 cuando se decremente el valor de idDepartment = 0 por uno
   * para obtener el indice del arreglo de conexiones, lo cual se hace para obtener
   * el descriptor de archivo del cliente al que otro cliente le quiere enviar algo
   */
  if ((idDepartment >= 1) && (idDepartment <= CONNECTION_LIMIT)) {
    return true;
  }

  return false;
}

// TODO: Añadir descripcion
static bool connectedClient(int idDepartment) {
  /*
   * Si el contenido de la posicion indicada por
   * idDepartment - 1 es distinto de FREE_CONNECTION (la cual
   * vale -1), entonces significa que el cliente solicitado
   * esta conectado al servidor, en este caso esta funcion
   * retorna verdadero, en caso contrario retorna falso
   */
  if (clients[idDepartment - 1] != FREE_CONNECTION) {
    return true;
  }

  return false;
}

// TODO: Añadir descripcion
static bool equalDepartment(int senderfd, int idDepartment) {

  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {

    if ((clients[i] == senderfd) && (i == idDepartment - 1)) {
      return true;
    }

  }

  return false;
}

// TODO: Documentar
static int getIdDepartment(int acceptfd) {

  for (size_t i = 0; i < CONNECTION_LIMIT; i++) {
    if (clients[i] == acceptfd) {
      return i + 1;
    }
  }

}
