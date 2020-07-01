#include <pthread.h>

// Comandos para encender y apagar las luces
void turnon(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void turnoff(int acceptfd,  bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

// Comandos para activar y desactivar el riego automatico
void ienable(int acceptfd,  bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void idisable(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

// Comando para la transmision y recepcion de imagenes
void simage(int senderfd, bool* sendDefaultMessage, const char* nameCommandBuf, int idDepartment, pthread_mutex_t lock, int clients[]);

void rimage(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

// Comandos para la emision y recepcion de llamadas
void takecall(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void callto(int senderfd, bool* sendDefaultMessage, const char* nameCommandBuf, int idDepartment, pthread_mutex_t lock, int clients[]);

// Comandos para la transmision y recepcion de audio
void sendaudio(int senderfd, int senderTcpFd, bool* sendDefaultMessage, const char* nameCommandBuf, int idDepartment, pthread_mutex_t lock, int clients[]);

void recaudio(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

// Otros comandos
void id(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void ping(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void disconnect(int acceptfd,  bool* sendDefaultMessage, bool* disconnection, const char* buf, int *amountConnections, pthread_mutex_t lock, int clients[]);
