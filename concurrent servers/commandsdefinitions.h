#include <pthread.h>

void turnon(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void turnoff(int acceptfd,  bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void ienable(int acceptfd,  bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void idisable(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void rimage(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void takecall(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void recaudio(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void id(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void ping(int acceptfd, bool* sendDefaultMessage, const char* buf, pthread_mutex_t lock, int clients[]);

void disconnect(int acceptfd,  bool* sendDefaultMessage, bool* disconnection, const char* buf, int *amountConnections, pthread_mutex_t lock, int clients[]);

void simage(int senderfd, bool* sendDefaultMessage, const char* nameCommandBuf, int idDepartment, pthread_mutex_t lock, int clients[]);

void callto(int senderfd, bool* sendDefaultMessage, const char* nameCommandBuf, int idDepartment, pthread_mutex_t lock, int clients[]);

void sendaudio(int senderfd, bool* sendDefaultMessage, const char* nameCommandBuf, int idDepartment, pthread_mutex_t lock, int clients[]);
