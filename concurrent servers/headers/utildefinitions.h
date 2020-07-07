#define CONNECTION_LIMIT 3
#define FREE_CONNECTION -1
#define BUF_SIZE 4096 /* Maximo tamaño del bufer para los flujos de datos que pueden leer el cliente y el servidor */

// int sendResultClientUdp(int acceptfd, char* answer, int numRead, const struct sockaddr *claddr, socklen_t len);

int sendResultClientUdp(int acceptfd, char* answer);

int sendResultClient(int acceptfd, char* answer);

int sendNoticeClientUdp(int senderUdpFd, char* answer, struct sockaddr_in addrSender);

// TODO: Este sera borrado despues de implmentar sendReplyServer()
void sendResultServer(int acceptfd, char* command, char* answer, int resultWrite, int clients[]);

void sendReplyServer(int senderDepartmentId, char* command, char* answer, int operationResult);

void sendNoticeReceiver(int acceptfd, int receivingDepartmentId, char* answer);

void invalidCommand(int acceptfd, int clients[]);

void responseInvalidCommand(int acceptfd, int clients[]);

bool existDepartment(int idDepartment);

bool connectedClient(int idDepartment, int clients[]);

bool equalDepartment(int givenSocketFd, int idDepartment, int arrayClients[]);

void printConnections(int *amountConnections);

int getIdDepartment(int clients[], int acceptfd);

void resetCharArray(char buf[]);

void addClient(int connectionfd, int socketUdpFd, int *amountConnections, pthread_mutex_t lock, int clients[], int clientsUdp[]);

// void addClient(int connectionfd, int *amountConnections, pthread_mutex_t lock, int clients[]);

void removeClient(int clients[], int clientsUdp[], int connectionfd, int *amountConnections);

void concatenateTextNotification(char* notification, char* firstPartNotification, char* secondPartNotification, int senderDepartmentId);

void fill(int clients[]);

char convertIntToChar(int number);

int getFdSocketTcp(char* ip, char* port);

int getFdSocketUdp(int acceptfd);

/**
 * @param  givenSocketFd
 * @return los datos (IP y puerto) del servidor asociados
 * al descriptor de archivo de un socket
 */
struct sockaddr_in getDataServer(int givenSocketFd);
