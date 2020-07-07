#include <pthread.h>

// Comandos para encender y apagar las luces
void turnon(int acceptfd, int clients[]);

void turnoff(int acceptfd, int clients[]);

// Comandos para activar y desactivar el riego automatico
void ienable(int acceptfd, int clients[]);

void idisable(int acceptfd, int clients[]);

// Comando para la transmision de un archivo desde el servidor hacia un cliente
void rimage(int acceptfd, int clients[]);

// Comandos para la emision y recepcion de llamadas
void takecall(int acceptfd, int clients[]);

void callto(int senderfd, int idDepartment, int clients[]);

// Comandos para la transmision de audio
void sendAudio(int senderUdpFd, int destinyDepartmentId, struct sockaddr_in addrSender, struct sockaddr_in addrReceiver, int clientsUdp[]);

// Otros comandos
void id(int acceptfd, int clients[]);

void ping(int acceptfd, int clients[]);

void disconnect(int acceptfd, bool* disconnection, int *amountConnections, pthread_mutex_t lock, int clients[], int clientsUdp[]);
