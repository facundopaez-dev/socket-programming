#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#define BUF_SIZE 4096

/**
 * Coloca en cada posicion del buffer enviado
 * como argumento, el caracter '\0' para eliminar
 * cualquier dato basura
 *
 * @param givenBuffer
 */
void resetBuffer(char givenBuffer[]);

/**
 * Muestra por pantalla el comando que fue ejecutado
 * por el usuario, comando que es pasado por argumento
 * a esta funcion cuando es invocada
 *
 * @param command
 */
void displayCommandExecuted(char* command);

/**
 * Muestra el contenido de lo que el programa cliente
 * le envia al servidor (con el cual esta conectado)
 * cuando el usuario introduce un comando valido
 *
 * @param sendBuffer
 */
void displayDataSent(char* sendBuffer);

/**
 * Esta funcion tiene la responsabilidad crear un socket
 * TCP haciendo uso de una IP y un puerto
 *
 * @param  ip
 * @param  port
 * @return el descriptor de archivo del socket TCP creado
 * con la IP y puerto pasados como argumento en la invocacion
 * de esta funcion
 */
int getFdSocketTcp(char* ip, char* port);

/**
 * Esta funcion tiene la responsabilidad de crear un socket
 * UDP haciendo uso de la IP y el puerto asociados al
 * descriptor de archivo de un socket TCP
 *
 * @param  socketTcpFd
 * @return el descriptor de archivo del socket UDP creado
 * con la IP y el puerto asociado al descriptor de archivo
 * de un socket TCP
 */
int getFdSocketUdp(int socketTcpFd);
